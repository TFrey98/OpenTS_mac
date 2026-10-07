/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#include "always.h"

#include "dbgprint.h"

#include "file.h"
#include "opents_build.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <crt_externs.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <pthread.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef _DEBUG
static char const BuildType[] = "debug";
#else
static char const BuildType[] = "release";
#endif

static char const DebugTruncationNotice[] = "\n*** Log size limit reached. Nothing further will be written to this file. ***\n";

static constexpr size_t DEBUG_MESSAGE_MAX = 4096;
static constexpr unsigned DEBUG_LOG_MAX_AGE_DAYS = 14;
static constexpr unsigned long long DEBUG_LOG_MAX_BYTES = 64ULL * 1024ULL * 1024ULL;
static constexpr unsigned long long DEBUG_LOG_NOTICE_RESERVE = sizeof(DebugTruncationNotice) - 1;
static constexpr unsigned long long DEBUG_LOG_BUDGET = DEBUG_LOG_MAX_BYTES - DEBUG_LOG_NOTICE_RESERVE;

static std::mutex DebugLock;
static std::atomic<pthread_t> DebugLockOwner{nullptr};
static bool DebugInitDone = false;
static bool AtLineStart = true;
static bool ConsoleActive = false;
static FILE * DebugFile = NULL;
static char DebugDirectory[PATH_MAX];
static char DebugFileName[PATH_MAX];
static unsigned long long DebugBytesWritten = 0;


/// <summary>
/// Reports whether the command line asks for the debug console. The game's own parser runs
/// too late to catch the messages written during early startup, so the process arguments are
/// read here instead.
/// </summary>
static bool Command_Line_Requests_Console(void)
{
	int const argc = *_NSGetArgc();
	char ** const argv = *_NSGetArgv();

	// Index zero is the executable path, which may itself look like an option.
	for (int index = 1; index < argc; index++) {
		char const * token = argv[index];
		if (token[0] != '-' || (token[1] != 'X' && token[1] != 'x')) {
			continue;
		}
		for (char const * code = token + 2; *code != '\0'; code++) {
			if (*code == 'C' || *code == 'c') {
				return(true);
			}
		}
	}
	return(false);
}


static void Local_Time(struct tm & parts, long & milliseconds)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	time_t const seconds = now.tv_sec;
	localtime_r(&seconds, &parts);
	milliseconds = (long)(now.tv_usec / 1000);
}


/// <summary>
/// Deletes files matching a pattern that were last written more than the given number of days
/// ago. Directories are never removed.
/// </summary>
/// <param name="directory">Directory to search, without a trailing separator.</param>
/// <param name="pattern">File name pattern, such as "DEBUG_*.LOG".</param>
/// <param name="days">Age threshold in days. Values above 90 are rejected.</param>
/// <returns>True if the directory was searched.</returns>
bool Delete_Files_Older_Than(char const * directory, char const * pattern, unsigned days)
{
	if (directory == NULL || pattern == NULL || days > 90) {
		return(false);
	}

	time_t const cutoff = time(NULL) - (time_t)days * 24 * 60 * 60;

	char search[PATH_MAX];
	snprintf(search, sizeof(search), "%s/%s", directory, pattern);

	for (FoundFileRecord const & file : Find_Files(search)) {
		if (Unix_Time_From_File_Time(file.WriteTime) < cutoff) {
			char victim[PATH_MAX];
			snprintf(victim, sizeof(victim), "%s/%s", directory, file.Name.c_str());
			unlink(victim);
		}
	}
	return(true);
}


/// <summary>
/// Mirrors the log to standard error, which is the terminal or Xcode's console when the game
/// was started from one. The caller holds the logging lock.
/// </summary>
static void Init_Console_Locked(void)
{
	ConsoleActive = true;
}


static void Write_Banner_Locked(struct tm const & started);


/// <summary>
/// Prepares the log directory and this run's log file, then turns on the console mirror if
/// this build or the command line asks for it. The caller holds the logging lock. A log that
/// cannot be opened leaves the console mirror working.
/// </summary>
static void Init_Locked(void)
{
	if (DebugInitDone) {
		return;
	}
	DebugInitDone = true;

	char const * home = getenv("HOME");
	std::error_code error;
	if (home != NULL && home[0] != '\0') {
		std::filesystem::path const directory = std::filesystem::path(home) / "Library/Logs/OpenTS";
		if (std::filesystem::create_directories(directory, error) || std::filesystem::is_directory(directory, error)) {
			snprintf(DebugDirectory, sizeof(DebugDirectory), "%s", directory.c_str());
		}
	}

	struct tm now;
	long milliseconds;
	Local_Time(now, milliseconds);

	char timestamp[32];
	strftime(timestamp, sizeof(timestamp), "%d-%m-%Y_%H-%M-%S", &now);

	if (DebugDirectory[0] != '\0') {
		Delete_Files_Older_Than(DebugDirectory, "DEBUG_*.LOG", DEBUG_LOG_MAX_AGE_DAYS);

		// "x" refuses an existing file, so a second process started in the same second does not
		// disturb the first one's log.
		snprintf(DebugFileName, sizeof(DebugFileName), "%s/DEBUG_%s.LOG", DebugDirectory, timestamp);
		DebugFile = fopen(DebugFileName, "wx");
		if (DebugFile == NULL) {
			snprintf(DebugFileName, sizeof(DebugFileName), "%s/DEBUG_%s_%d.LOG", DebugDirectory, timestamp, (int)getpid());
			DebugFile = fopen(DebugFileName, "wx");
		}
		if (DebugFile == NULL) {
			DebugFileName[0] = '\0';
		}
	}

#ifdef _DEBUG
	Init_Console_Locked();
#else
	if (Command_Line_Requests_Console()) {
		Init_Console_Locked();
	}
#endif

	// Last, so that the banner heads the log and also reaches the console.
	Write_Banner_Locked(now);
}


/// <summary>
/// Writes raw text to every enabled sink. The caller holds the logging lock.
/// </summary>
static void Write_Text_Locked(char const * text, size_t length)
{
	if (DebugFile != NULL) {

		// The notice is paid for out of the reserve, so the file never passes its limit.
		if (DebugBytesWritten + length > DEBUG_LOG_BUDGET) {
			fwrite(DebugTruncationNotice, 1, DEBUG_LOG_NOTICE_RESERVE, DebugFile);
			fclose(DebugFile);
			DebugFile = NULL;
		} else {
			fwrite(text, 1, length, DebugFile);
			fflush(DebugFile);
			DebugBytesWritten += length;
		}
	}

	if (ConsoleActive) {
		fwrite(text, 1, length, stderr);
	}
}


/// <summary>
/// Writes one finished message, stamping the record prefix when one is due. The caller holds
/// the logging lock.
/// </summary>
static void Write_Message_Locked(char const * buffer, bool with_prefix)
{
	size_t const length = strlen(buffer);
	if (length == 0) {
		return;
	}

	// The prefix identifies a record rather than a call, so it is written once per line: a
	// line assembled from several calls is stamped where it starts. Prefix and message go out
	// together to keep this to one write per call.
	if (with_prefix && AtLineStart) {
		struct tm now;
		long milliseconds;
		Local_Time(now, milliseconds);

		char stamped[DEBUG_MESSAGE_MAX + 32];
		int const written = snprintf(stamped, sizeof(stamped), "[%02d:%02d:%02d.%03ld] %s",
												now.tm_hour, now.tm_min, now.tm_sec, milliseconds, buffer);
		if (written > 0) {
			// snprintf reports the length it wanted, which is not what was stored.
			size_t const kept = std::min(size_t(written), sizeof(stamped) - 1);
			Write_Text_Locked(stamped, kept);
			AtLineStart = buffer[length - 1] == '\n';
			return;
		}
	}

	Write_Text_Locked(buffer, length);
	AtLineStart = buffer[length - 1] == '\n';
}


/// <summary>
/// Opens the log with the wordmark and the facts that identify the build and the run. The
/// caller holds the logging lock, so the text goes through the unlocked sink directly;
/// DebugStringNoPrefix would meet the re-entrancy guard and be dropped.
/// </summary>
/// <param name="started">The time this run's log was opened.</param>
static void Write_Banner_Locked(struct tm const & started)
{
	// A raw literal keeps the lettering readable, and keeps its backslashes out of the reach of
	// escape processing. It opens on its own line so the rows line up here, which costs a
	// leading newline that the write below steps over.
	static char const Wordmark[] =
R"ART(
  ___                  _____ ____
 / _ \ _ __   ___ _ __|_   _/ ___|
| | | | '_ \ / _ \ '_ \ | | \___ \
| |_| | |_) |  __/ | | || |  ___) |
 \___/| .__/ \___|_| |_||_| |____/
      |_|

)ART";

	Write_Message_Locked(Wordmark + 1, false);

	char line[512];

	snprintf(line, sizeof(line), "Version  : OpenTS %s (%s %s build)\n", OPENTS_VERSION, OPENTS_ARCH, BuildType);
	Write_Message_Locked(line, false);

	snprintf(line, sizeof(line), "Commit   : %s on %s%s\n", OPENTS_COMMIT, OPENTS_BRANCH,
				OPENTS_COMMIT_DIRTY ? " (modified)" : "");
	Write_Message_Locked(line, false);

	snprintf(line, sizeof(line), "Committed: %s\n", OPENTS_COMMIT_DATE);
	Write_Message_Locked(line, false);

	char started_text[32];
	strftime(started_text, sizeof(started_text), "%Y-%m-%d %H:%M:%S", &started);
	snprintf(line, sizeof(line), "Started  : %s\n", started_text);
	Write_Message_Locked(line, false);

	char version[64] = "unknown";
	char build[64] = "";
	size_t size = sizeof(version);
	sysctlbyname("kern.osproductversion", version, &size, NULL, 0);
	size = sizeof(build);
	sysctlbyname("kern.osversion", build, &size, NULL, 0);
	snprintf(line, sizeof(line), "System   : macOS %s (%s)\n", version, build);
	Write_Message_Locked(line, false);

	// The arguments only. The executable path usually carries the account name.
	char options[256] = "(none)";
	int const argc = *_NSGetArgc();
	char ** const argv = *_NSGetArgv();
	size_t used = 0;
	for (int index = 1; index < argc; index++) {
		int const written = snprintf(options + used, sizeof(options) - used, "%s%s", used == 0 ? "" : " ", argv[index]);
		if (written <= 0 || size_t(written) >= sizeof(options) - used) {
			break;
		}
		used += size_t(written);
	}

	snprintf(line, sizeof(line), "Options  : %s\n", options);
	Write_Message_Locked(line, false);

	Write_Message_Locked("--------------------------------------------------------------------------------\n", false);
}


/// <summary>
/// Takes the logging lock and reports one finished message.
/// </summary>
static void Emit(char const * buffer, bool with_prefix)
{
	pthread_t const self = pthread_self();

	// A fault raised inside a logging call brings the handler back here on the same thread,
	// where taking the lock again would deadlock. Such a message goes to standard error only.
	if (DebugLockOwner.load() == self) {
		fputs(buffer, stderr);
		return;
	}

	std::lock_guard<std::mutex> hold(DebugLock);
	DebugLockOwner = self;

	Init_Locked();
	Write_Message_Locked(buffer, with_prefix);

	DebugLockOwner = nullptr;
}


/// <summary>
/// Runs first time initialisation under the logging lock.
/// </summary>
static void Init_Once(bool with_console)
{
	std::lock_guard<std::mutex> hold(DebugLock);
	DebugLockOwner = pthread_self();

	Init_Locked();
	if (with_console) {
		Init_Console_Locked();
	}

	DebugLockOwner = nullptr;
}


/// <summary>
/// Prepares the debug log and, when the build or the command line asks for it, the console
/// mirror. Logging works without this call, but calling it early fixes the log's timestamp
/// at process start.
/// </summary>
void Debug_Init(void)
{
	Init_Once(false);
}


/// <summary>
/// Mirrors the log to standard error if it is not mirrored already.
/// </summary>
void Debug_Init_Console(void)
{
	Init_Once(true);
}


/// <summary>
/// Waits for Return when the console mirror is on and standard input is a terminal, so that
/// text written just before the process exits stays readable. Does nothing otherwise.
/// </summary>
void Debug_Console_Hold(void)
{
	if (!ConsoleActive || !isatty(STDIN_FILENO)) {
		return;
	}

	DebugString("Press Return to exit.\n");
	getchar();
}


/// <summary>
/// Returns the full path of this run's debug log, or an empty string when no log could be
/// opened. Intended for startup code and user interface text; never call it from a crash
/// handler, because it takes the logging lock.
/// </summary>
char const * Debug_Log_File_Name(void)
{
	Init_Once(false);
	return(DebugFileName);
}


/// <summary>
/// Returns the folder where per-run diagnostic files belong, ~/Library/Logs/OpenTS, or an
/// empty string when it could not be created. Shared by callers that write their own files
/// beside the debug log.
/// </summary>
char const * Debug_Directory(void)
{
	Init_Once(false);
	return(DebugDirectory);
}


/// <summary>
/// Reports a formatted message to the debug log and the console mirror. A message that starts
/// a line is stamped with the time it was reported.
/// </summary>
/// <param name="string">The printf style format string to report.</param>
void DebugString(char const * string, ...)
{
	// Callers report an error and then branch on it, so logging must not disturb it.
	int const last_errno = errno;

	char buffer[DEBUG_MESSAGE_MAX];

	va_list va;
	va_start(va, string);
	vsnprintf(buffer, sizeof(buffer), string, va);
	va_end(va);

	Emit(buffer, true);

	errno = last_errno;
}


/// <summary>
/// Reports a formatted message with no identifying prefix, so the text appears exactly as
/// given. Callers use it to continue a line another call began, and for text such as the
/// startup banner that reads better unstamped. It reaches the same places DebugString does.
/// </summary>
/// <param name="string">The printf style format string to report.</param>
void DebugStringNoPrefix(char const * string, ...)
{
	int const last_errno = errno;

	char buffer[DEBUG_MESSAGE_MAX];

	va_list va;
	va_start(va, string);
	vsnprintf(buffer, sizeof(buffer), string, va);
	va_end(va);

	Emit(buffer, false);

	errno = last_errno;
}


/// <summary>
/// Returns the system message text for an errno value, in a buffer owned by the calling thread.
/// </summary>
/// <param name="error">An errno value.</param>
char const * Last_Error_Text(unsigned long error)
{
	static thread_local char message_buffer[256];

	if (strerror_r((int)error, message_buffer, sizeof(message_buffer)) != 0) {
		message_buffer[0] = '\0';
	}

	return(message_buffer);
}
