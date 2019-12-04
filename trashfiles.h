#pragma once
#include <string>

// http://www.ramendik.ru/docs/trashspec.html
// https://www.codeproject.com/articles/527455/extending-boost-filesystem-for-windows-and-linux-2
// Trash info file https://stackoverflow.com/questions/17964439/move-files-to-trash-recycle-bin-in-qt

class TrashFiles {
 public:
  TrashFiles();

  enum class TrashResult { SUCCESS, ERROR, MISSING_FILE, };
  TrashResult TrashOneFile(const char* path);
  bool isOk();

  //const std::string& TrashDir() const { return m_trashdir; }
  //const std::string& FileDir()  const { return m_filedir; }
  //const std::string& InfoDir()  const { return m_infodir; }

 private:
  std::string m_trashdir,
	      m_filedir,
	      m_infodir;

  class TrashInfofile {
  public:
    //TrashInfofile() { }
    ~TrashInfofile() { close(); }

    bool open(std::string& path) {
      return (fp = fopen(path.c_str(), "wx")) != nullptr;
    }
    void close() {
      if (fp) fclose(fp);
      fp = nullptr;
    }

    bool isOpen() const { return fp != nullptr; }
    operator FILE*() const { return fp; }

  private:
    FILE* fp = nullptr;
  };
};
