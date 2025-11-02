// (c) Beem Media. All rights reserved.

#include "lf_lpk.h"
#include "lf_sys2.h"

#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

template<typename Fmt, typename ... Args>
static constexpr void LFPak_Log(const Fmt& Format, Args ... args)
{
	const std::string LogText = std::vformat(Format, std::make_format_args(args...));
	std::cout << LogText << std::endl;
}

static int LFPak_Extract(const char* Filename);
static int LFPak_Compact(const char* Directory, const char* OutFilename);

int main(int argc, const char* argv[])
{
	LFPak_Log("Legacy File System PaKage Utility (c) Beem Media.");

	bool bNextIsExtract = false;
	bool bNextIsCompact = false;
	bool bNextIsCompactOut = false;

	std::string CompactDir;
	std::string CompactFile;
	
	for (int i = 0; i < argc; i++)
	{
		std::string Arg = argv[i];

		if (bNextIsExtract)
		{
			return LFPak_Extract(argv[i]);
		}
		else if (bNextIsCompact)
		{
			CompactDir = argv[i];
			bNextIsCompact = false;
		}
		else if (bNextIsCompactOut)
		{
			CompactFile = argv[i];
			bNextIsCompactOut = false;
		}
		else if (Arg == "-e" || Arg == "-extract")
		{
			bNextIsExtract = true;
		}
		else if (Arg == "-c" || Arg == "-compact")
		{
			bNextIsCompact = true;
		}
		else if (Arg == "-f" || Arg == "-file")
		{
			bNextIsCompactOut = true;
		}
	}

	if (CompactDir.size() > 0 && CompactFile.size() > 0)
	{
		return LFPak_Compact(CompactDir.c_str(), CompactFile.c_str());
	}

	LFPak_Log("Usage: ");
	LFPak_Log("  lf_pak -e Filename");
	LFPak_Log("  lf_pak -c $DirectoryPath -f $OutFilename");
	LFPak_Log("-e will extract all contents of a lpk file.");
	LFPak_Log("-c will compact a directory and save it to $OutFilename.");
	return -1;
}

static int LFPak_Extract(const char* Filename)
{
	const std::filesystem::path AsPath(Filename);
	const std::filesystem::path RootDir = std::filesystem::path(AsPath).replace_extension();

	CLArchive Archive;
	Archive.OpenA(Filename);
	if (!Archive.IsOpen())
	{
		LFPak_Log("Not a valid lpk file.");
		return -1;
	}

	const lf_dword NumFiles = Archive.GetNumFiles();
	LFPak_Log("Extracting {0} files to {1}...", NumFiles, RootDir.string());

	std::vector<lf_byte> DataCache;

	for (lf_dword i = 0; i < NumFiles; i++)
	{
		if (const LPK_FILE_INFO* Info = Archive.GetFileInfo(i))
		{
			std::filesystem::path ExtractTo = RootDir / Info->szFilename;// LfPak_AppendPath(RootDir, FileParts);

			LFPak_Log("  Extracting {0}...", ExtractTo.string());

			const std::filesystem::path Folder = ExtractTo.parent_path();
			const bool bDirExists = std::filesystem::is_directory(Folder);
			const bool bDirCreated = bDirExists || std::filesystem::create_directories(Folder);
			if (bDirCreated)
			{
				DataCache.resize(Info->nSize);
				Archive.ExtractFile(DataCache.data(), i);

				std::ofstream OutFile(ExtractTo, std::ios::out | std::ios::binary);
				if (OutFile.is_open())
				{
					OutFile.write(reinterpret_cast<const char*>(DataCache.data()), DataCache.size());
				}
			}
		}
	}

	return 0;
}

static int LFPak_Compact(const char* Directory, const char* OutFilename)
{
	const std::filesystem::path AsPath(Directory);

	CLArchive Archive;
	Archive.CreateNewA(OutFilename);
	if (!Archive.IsOpen())
	{
		LFPak_Log("Could not open file for writing.");
		return -1;
	}

	for (const auto& Item : std::filesystem::recursive_directory_iterator(AsPath))
	{
		if (std::filesystem::is_regular_file(Item.status()))
		{
			const std::filesystem::path Relative = std::filesystem::relative(Item.path(), AsPath);

			LFPak_Log("  Adding: {0}...", Relative.generic_string());

			Archive.AddFileA(Item.path().generic_string().c_str(), Relative.generic_string().c_str(), LPK_ADD_ZLIBCMP);
		}
	}

	return -1;
}
