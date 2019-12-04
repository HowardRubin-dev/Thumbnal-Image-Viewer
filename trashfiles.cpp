#include "trashfiles.h"
#include "utility.h"
//#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>


TrashFiles::TrashFiles() {
  char* dir;

  if ((dir = getenv("XDG_DATA_HOME")) != nullptr && dir[0] != '\0') {
    m_trashdir = dir;
    m_trashdir += "/Trash/";
  } else {
    m_trashdir = TivUtility::HomeDir() + "/.local/share/Trash/";
  }

  m_filedir = m_trashdir + "files/";
  m_infodir = m_trashdir + "info/";

  // In case directories don't already exist. Tested doesn't overwrite existing.
  mkdir(m_trashdir.c_str(), S_IRWXU);
  mkdir(m_filedir.c_str(),  S_IRWXU);
  mkdir(m_infodir.c_str(),  S_IRWXU);
}

bool TrashFiles::isOk() {
  struct stat st;
  if (stat(m_filedir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)
   && stat(m_infodir.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    return true;
  else
    return false;
}

TrashFiles::TrashResult TrashFiles::TrashOneFile(const char* originalPath) {
  std::string originalFilename = TivUtility::Basename(originalPath);
  std::string trashinfoPath,
	      deletedFilePath;
  // Trash whole folder not needed for tiv program but could use system("mv...")

  if (access(originalPath, R_OK) != 0)
    return TrashResult::MISSING_FILE; // Source missing

  TrashInfofile infoFile;

  // Get *.trashinfo file name and open it
  for (int i=1; i<1000; ++i) {
    if (i == 1) {
      deletedFilePath = m_filedir + originalFilename;
      trashinfoPath   = m_infodir + originalFilename + ".trashinfo";
    } else {
      // TODO? cut.2.png not cut.png.2
      deletedFilePath = m_filedir + originalFilename + "." + std::to_string(i);
      trashinfoPath   = m_infodir + originalFilename + "." + std::to_string(i) + ".trashinfo";
    }

    if (access(trashinfoPath.c_str(), R_OK) == 0)
      continue; // File already exists

    if (infoFile.open(trashinfoPath)) {
      break; // Good
    } else if (access(trashinfoPath.c_str(), R_OK) != 0) {
      return TrashResult::ERROR; // File doesn't exist and couldn't open new
      // TODO someone restored the trashed file asynchronously
    } else {
      continue; // Someone else opened the file before we could. Try again.
    }
  }

  if (!infoFile.isOpen())
    return TrashResult::ERROR; // Couldn't open the trash info file

/*
[Trash Info]
Path=/home/xxxx/Pictures/crap
DeletionDate=2019-10-25T20:47:50
*/
  time_t t;
  time(&t);
  struct tm* info = localtime(&t);

  fprintf(infoFile, "[Trash Info]\n"
	  "Path=%s\n"
	  "DeletionDate=%d-%02d-%02dT%02d:%02d:%02d\n", originalPath,
	  info->tm_year+1900, info->tm_mon+1, info->tm_mday,
	  info->tm_hour,      info->tm_min,   info->tm_sec);
  infoFile.close();

  if (rename(originalPath, deletedFilePath.c_str()) == 0)
    return TrashResult::SUCCESS;

  // TODO? if (errno == EXDEV) ...
  // Error moving original file so copy and delete original
  std::ifstream src(originalPath,    std::ifstream::binary);
  std::ofstream dst(deletedFilePath, std::ofstream::binary);
  dst << src.rdbuf();

  if (access(deletedFilePath.c_str(), R_OK) != 0) {
    unlink(trashinfoPath.c_str()); // File not copied, delete trashinfo file
    return TrashResult::ERROR;
  }

  unlink(originalPath); // Delete original

  return TrashResult::SUCCESS;
}
