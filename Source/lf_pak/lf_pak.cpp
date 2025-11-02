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
static int LFPak_Compact(const char* Directory);

int main(int argc, const char* argv[])
{
	LFPak_Log("Legacy File System PaKage Utility (c) Beem Media.");

	bool bNextIsExtract = false;
	bool bNextIsCompact = false;
	
	for (int i = 0; i < argc; i++)
	{
		std::string Arg = argv[i];

		if (bNextIsExtract)
		{
			return LFPak_Extract(argv[i]);
		}
		else if (bNextIsCompact)
		{
			return LFPak_Compact(argv[i]);
		}
		else if (Arg == "-e" || Arg == "-extract")
		{
			bNextIsExtract = true;
		}
		else if (Arg == "-c" || Arg == "-compact")
		{
			bNextIsCompact = true;
		}
	}

	LFPak_Log("Usage: ");
	LFPak_Log("  lf_pak -e Filename");
	LFPak_Log("  lf_pak -c $DirectoryPath");
	LFPak_Log("-e will extract all contents of a lpk file.");
	LFPak_Log("-c will compact a directory.");
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

static int LFPak_Compact(const char* Directory)
{
	LFPak_Log("Compact is not implemented.");
	return -1;
}
