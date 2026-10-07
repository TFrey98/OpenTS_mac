/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "gamedirs.h"

#include "cdfile.h"
#include "choosefolder.h"
#include "dbgprint.h"
#include "file.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits.h>
#include <mach-o/dyld.h>
#include <strings.h>
#include <vector>

/*
 * The directories the command line named. Empty means the game's own directory, so an
 * installation that names neither resolves every file exactly as it always did.
 */
static std::string DataDirectory;
static std::string UserDirectory;

/*
 * The folder saved games are kept in, under whichever directory the player's own files go.
 */
static char const * const SavedGamesFolder = "Saved Games";

static char const * const ScreenshotsFolder = "Screenshots";


static std::string Trim_Path(std::string const & path)
{
	std::string::size_type first = path.find_first_not_of(" \t");
	if (first == std::string::npos) {
		return(std::string());
	}

	std::string::size_type last = path.find_last_not_of(" \t");
	return(path.substr(first, last - first + 1));
}


/// <summary>
/// Puts a directory in the form a file name can simply be appended to, which is what the
/// search chain has always expected of one.
/// </summary>
/// <param name="path">The directory to terminate.</param>
/// <returns>The directory, ending in a separator.</returns>
static std::string Terminate_Path(std::string const & path)
{
	if (path.empty()) {
		return(path);
	}

	switch (path[path.length() - 1]) {
		case '\\':
		case '/':
		case ':':
			return(path);

		default:
			return(path + (char)std::filesystem::path::preferred_separator);
	}
}


static bool Is_Same_Path(std::string const & left, std::string const & right)
{
	return(strcasecmp(left.c_str(), right.c_str()) == 0);
}


static bool Is_Registered(std::string const & path)
{
	for (int index = 0; ; index++) {
		char const * registered = CDFileClass::Search_Path(index);
		if (registered == NULL) {
			return(false);
		}

		if (Is_Same_Path(registered, path)) {
			return(true);
		}
	}
}


std::string Data_Directory(void)
{
	return(DataDirectory);
}


/*
 * What went wrong with the directories, kept for whoever is in a position to tell the
 * player. Reporting it is not this module's business, since it has no window to report in.
 */
static std::string DirectoryError;


static void Report_Directory_Error(char const * what, std::string const & path)
{
	char message[MAX_PATH + 128];

	sprintf(message, "The %s directory cannot be used:\n\n%s", what, path.c_str());
	DirectoryError = message;

	DebugString("[GameDirs] %s directory unusable: %s.\n", what, path.c_str());
	printf("The %s directory cannot be used: %s\n", what, path.c_str());
}


char const * Game_Directory_Error(void)
{
	return(DirectoryError.c_str());
}


static bool Is_Directory(std::string const & path)
{
	std::error_code error;
	return(std::filesystem::is_directory(path, error));
}


void Set_Data_Directory(char const * path)
{
	DataDirectory = Terminate_Path(Trim_Path(path != NULL ? path : ""));
}


void Set_User_Directory(char const * path)
{
	UserDirectory = Terminate_Path(Trim_Path(path != NULL ? path : ""));

	// The file layer places and finds the player's own files; this is the only thing that
	// tells it where they go.
	CDFileClass::Set_User_Path(UserDirectory.c_str());
}


/// <summary>
/// Splits a configured folder list into the folders it names.
/// Folders are separated by commas, since a semicolon opens a comment in the file the list
/// is written in. They are returned in the order written, with the whitespace around them
/// dropped and a trailing separator supplied, and a folder named twice is kept once.
/// </summary>
/// <param name="list">The comma separated list of folders.</param>
/// <returns>The folders named, in the order they were written.</returns>
std::vector<std::string> Parse_Search_Folders(char const * list)
{
	std::vector<std::string> folders;

	if (list == NULL) {
		return(folders);
	}

	std::string const text = list;
	std::string::size_type start = 0;

	while (start <= text.length()) {
		std::string::size_type end = text.find(',', start);
		if (end == std::string::npos) {
			end = text.length();
		}

		/*
		 * Comparing the folders only once they are in the form they will be searched in
		 * keeps the same folder written two ways from being searched twice.
		 */
		std::string const folder = Terminate_Path(Trim_Path(text.substr(start, end - start)));

		/*
		 * The game's own directory is examined before any of these, so naming it adds
		 * nothing. Naming only it is how a deployment asks for no other folder, an entry
		 * with nothing after the equals sign being one an INI file cannot carry.
		 */
		if (folder == ".\\" || folder == "./") {
			if (end == text.length()) {
				break;
			}
			start = end + 1;
			continue;
		}

		if (!folder.empty()) {
			bool present = false;
			for (std::string const & existing : folders) {
				if (Is_Same_Path(existing, folder)) {
					present = true;
					break;
				}
			}

			if (!present) {
				folders.push_back(folder);
			}
		}

		if (end == text.length()) {
			break;
		}
		start = end + 1;
	}

	return(folders);
}


// The folder chosen for the game data, kept in the user directory so later launches find it.
static char const DATA_FOLDER_RECORD[] = "data-folder.txt";


// A folder holds the game when TIBSUN.MIX is in it or in its MIX folder, whatever the case
// of the names.
static bool Holds_Game_Data(std::filesystem::path const & folder)
{
	std::error_code error;
	for (std::filesystem::path const & place : {folder, folder / "MIX"}) {
		for (std::filesystem::directory_entry const & entry : std::filesystem::directory_iterator(place, error)) {
			if (strcasecmp(entry.path().filename().c_str(), "TIBSUN.MIX") == 0) {
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Finds the game data when the command line named no data directory: the folder chosen on an
/// earlier launch, ~/Library/Application Support/OpenTS/Data, the folder holding the application
/// bundle, and the executable's own folder, in that order. When none holds the game, the player
/// is asked to choose a folder until one that does is chosen or the panel is cancelled. The
/// folder found is remembered in the user directory.
/// </summary>
/// <returns>The folder holding the game, or an empty string when the player cancelled.</returns>
static std::string Find_Game_Data(void)
{
	std::filesystem::path const record = std::filesystem::path(UserDirectory) / DATA_FOLDER_RECORD;
	std::vector<std::filesystem::path> candidates;

	std::ifstream saved(record);
	std::string line;
	if (saved && std::getline(saved, line) && !line.empty()) {
		candidates.push_back(line);
	}
	candidates.push_back(std::filesystem::path(UserDirectory) / "Data");

	char executable[PATH_MAX];
	uint32_t size = sizeof(executable);
	if (_NSGetExecutablePath(executable, &size) == 0) {
		std::error_code error;
		std::filesystem::path const program = std::filesystem::canonical(executable, error);
		// Contents/MacOS/OpenTS inside OpenTS.app, whose own folder is three levels up.
		std::filesystem::path const bundle = program.parent_path().parent_path().parent_path();
		if (bundle.extension() == ".app") {
			candidates.push_back(bundle.parent_path());
		}
		candidates.push_back(program.parent_path());
	}

	std::string found;
	for (std::filesystem::path const & candidate : candidates) {
		if (Holds_Game_Data(candidate)) {
			found = candidate.string();
			break;
		}
	}

	while (found.empty()) {
		std::string const chosen = Choose_Folder("Choose the folder that holds Tiberian Sun's game files, such as TIBSUN.MIX.");
		if (chosen.empty()) {
			return(std::string());
		}
		if (Holds_Game_Data(chosen)) {
			found = chosen;
		} else {
			Show_Alert("Tiberian Sun's game files were not found",
				"That folder does not hold TIBSUN.MIX. Choose the folder the game's .MIX files were copied to.");
		}
	}

	std::ofstream(record) << found << '\n';
	return(found);
}


/// <summary>
/// Makes the directories the command line named usable, finding the game data when no data
/// directory was named. Without a named user directory, the player's files go to ~/Library/Application
/// Support/OpenTS, since the application bundle the game runs from must not change. The user
/// directory is created when it is not there yet, because it is the game's own to write. A
/// named data directory must already exist, a missing one being reported here rather than as
/// the missing files it would become later.
/// </summary>
/// <returns>bool; Can the game run with the directories it was given?</returns>
bool Apply_Game_Directories(void)
{
	char const * home = getenv("HOME");
	if (UserDirectory.empty() && home != NULL && home[0] != '\0') {
		Set_User_Directory((std::filesystem::path(home) / "Library/Application Support/OpenTS").c_str());
	}

	if (!UserDirectory.empty()) {
		std::error_code error;
		// create_directories answers false for a path ending in a separator even when it made
		// the folder, so the folder itself is checked.
		std::filesystem::create_directories(UserDirectory, error);
		if (!Is_Directory(UserDirectory)) {
			Report_Directory_Error("user", UserDirectory);
			return(false);
		}

		DebugString("[GameDirs] User directory is %s.\n", UserDirectory.c_str());
	}

	if (DataDirectory.empty()) {
		std::string const found = Find_Game_Data();
		if (found.empty()) {
			DebugString("[GameDirs] No game data folder was chosen.\n");
			return(false);
		}
		Set_Data_Directory(found.c_str());
	}

	if (!DataDirectory.empty()) {
		if (!Is_Directory(DataDirectory)) {
			Report_Directory_Error("data", DataDirectory);
			return(false);
		}

		CDFileClass::Add_Search_Drive(DataDirectory.c_str());
		DebugString("[GameDirs] Data directory is %s.\n", DataDirectory.c_str());
	}

	return(true);
}


void Init_Search_Folders(char const * list)
{
	std::string const home = Data_Directory();

	for (std::string const & folder : Parse_Search_Folders(list)) {
		std::string const path = home + folder;

		if (!Is_Registered(path)) {
			CDFileClass::Add_Search_Drive(path.c_str());
			DebugString("[GameDirs] Searching %s.\n", path.c_str());
		}
	}
}


std::string User_File_Write_Name(char const * filename)
{
	if (UserDirectory.empty()) {
		return(filename);
	}

	return(UserDirectory + filename);
}


static std::string Own_Folder_Name(char const * folder, char const * filename)
{
	std::string const path = UserDirectory + folder;

	std::error_code error;
	std::filesystem::create_directory(path, error);

	return(path + (char)std::filesystem::path::preferred_separator + filename);
}


/// <summary>
/// Names a saved game inside the folder they are kept in. The folder is not one of the
/// searched ones and is created here, so a launcher can browse it before the first save is
/// written.
/// </summary>
/// <returns>The name to open, delete or scan for.</returns>
std::string Saved_Game_Name(char const * filename)
{
	return(Own_Folder_Name(SavedGamesFolder, filename));
}


/// <summary>
/// Names a screen capture inside the Screenshots folder, making the folder on every call.
/// </summary>
std::string Screenshot_Name(char const * filename)
{
	return(Own_Folder_Name(ScreenshotsFolder, filename));
}


static void Scan_Folder(char const * prefix, char const * pattern, std::vector<std::string> & names)
{
	std::string const search = std::string(prefix) + pattern;

	for (FoundFileRecord const & file : Find_Files(search.c_str())) {
		bool present = false;
		for (std::string const & existing : names) {
			if (Is_Same_Path(existing, file.Name)) {
				present = true;
				break;
			}
		}

		if (!present) {
			names.push_back(file.Name);
		}
	}
}


/// <summary>
/// Finds the files matching a pattern in every directory the game reads from.
/// A name held by more than one directory is reported once, and opening that name afterwards
/// lands on the same file this scan saw, because both walk the directories in the same order.
/// The names come back sorted, so the order does not depend on the file system.
/// </summary>
/// <param name="pattern">The wildcard pattern to match, with no directory attached.</param>
/// <returns>The matching file names, without the directory they were found in.</returns>
std::vector<std::string> Search_Files(char const * pattern)
{
	std::vector<std::string> names;

	/*
	 * Asked of the file layer rather than kept here, so that a scan and an open are reading
	 * the very same directory.
	 */
	char const * user = CDFileClass::User_Path();
	if (user != NULL) {
		Scan_Folder(user, pattern, names);
	}

	Scan_Folder("", pattern, names);

	for (int index = 0; ; index++) {
		char const * path = CDFileClass::Search_Path(index);
		if (path == NULL) {
			break;
		}

		Scan_Folder(path, pattern, names);
	}

	std::sort(names.begin(), names.end(), [](std::string const & left, std::string const & right) {
		return(strcasecmp(left.c_str(), right.c_str()) < 0);
	});

	return(names);
}
