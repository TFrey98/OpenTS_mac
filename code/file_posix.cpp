/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2020-2024 Vanilla Conquer contributors
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Vanilla Conquer
 * (https://github.com/TheAssemblyArmada/Vanilla-Conquer).
 * Modified by OpenTS contributors, 2026.
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "always.h"

#include "file.h"

#include <cstring>
#include <memory>
#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class Find_File_Data_Posix : public Find_File_Data
{
	public:
		Find_File_Data_Posix();
		virtual ~Find_File_Data_Posix();

		virtual const char * GetName() const;
		virtual const char * GetFullName() const { return DirEntry != nullptr ? FullName : nullptr; }
		virtual unsigned int GetTime() const;

		virtual bool FindFirst(const char * fname);
		virtual bool FindNext();
		virtual void Close();

	private:
		DIR * Directory;
		struct dirent * DirEntry;
		char FileFilter[PATH_MAX];
		char FullName[PATH_MAX];
		char DirName[PATH_MAX];

		bool FindNextWithFilter();
};

Find_File_Data_Posix::Find_File_Data_Posix() : Directory(nullptr), DirEntry(nullptr), FileFilter{} {}

Find_File_Data_Posix::~Find_File_Data_Posix()
{
	Close();
}

const char * Find_File_Data_Posix::GetName() const
{
	if (DirEntry == nullptr) {
		return nullptr;
	}
	return DirEntry->d_name;
}

unsigned int Find_File_Data_Posix::GetTime() const
{
	if (DirEntry == nullptr) {
		return 0;
	}
	struct stat buf = {0};
	if (stat(FullName, &buf) != 0) {
		return 0;
	}
	return buf.st_mtime;
}

bool Find_File_Data_Posix::FindNextWithFilter()
{
	while (true) {
		DirEntry = readdir(Directory);
		if (DirEntry == nullptr) {
			return false;
		}
		if (fnmatch(FileFilter, DirEntry->d_name, FNM_PATHNAME | FNM_CASEFOLD) == 0) {
			strcpy(FullName, DirName);
			strcat(FullName, DirEntry->d_name);
			break;
		}
	}
	return true;
}

bool Find_File_Data_Posix::FindFirst(const char * fname)
{
	Close();
	FullName[0] = '\0';
	DirName[0] = '\0';

	// split directory and file from the path
	char * fdir = strrchr((char *)fname, '/');
	if (fdir != nullptr) {
		strncat(DirName, fname, (fdir - fname + 1));
		snprintf(FileFilter, sizeof(FileFilter), "%s", fdir + 1);
		Directory = opendir(DirName);
	} else {
		snprintf(FileFilter, sizeof(FileFilter), "%s", fname);
		Directory = opendir(".");
	}

	if (Directory == nullptr) {
		return false;
	}

	return FindNextWithFilter();
}

bool Find_File_Data_Posix::FindNext()
{
	if (Directory == nullptr) {
		return false;
	}
	return FindNextWithFilter();
}

void Find_File_Data_Posix::Close()
{
	if (Directory != nullptr) {
		closedir(Directory);
		Directory = nullptr;
	}
}

Find_File_Data * Find_File_Data::CreateFindData()
{
	return new Find_File_Data_Posix();
}

std::vector<FoundFileRecord> Find_Files(char const * pattern)
{
	std::vector<FoundFileRecord> found;
	std::unique_ptr<Find_File_Data> search(Find_File_Data::CreateFindData());
	if (!search->FindFirst(pattern)) {
		return(found);
	}
	do {
		struct stat info;
		char const * name = search->GetName();
		if (name[0] == '.' || stat(search->GetFullName(), &info) != 0 || !S_ISREG(info.st_mode)) {
			continue;
		}
		found.push_back({name, File_Time_From_Unix(info.st_mtimespec.tv_sec, info.st_mtimespec.tv_nsec)});
	} while (search->FindNext());
	search->Close();
	return(found);
}

