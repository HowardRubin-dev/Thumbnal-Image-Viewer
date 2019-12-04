#pragma once
#include <QStandardItem>

class CatalogItem : public QStandardItem {
  //Q_OBJECT QStandardItem doesn't inherit from QObject

 public:
  static CatalogItem* CreateFromImageFile(const char* path);
  static CatalogItem* CreateFromCatalog(std::ifstream& ifsCatalogFile);
  bool UpdateFromFile();
  void WriteToCatalog(std::ofstream& ofsCatalogFile) const;

  //bool Changed(const struct stat& st) const;
  bool Changed() const;

  time_t ModTime() const { return imageModTime; }
  //off_t ImageSize() const { return imageSizeOriginal; };

  const std::string& ImagePath() const { return sImagePath; }

  bool bCurrent, bSelected; // To preserve selections after sort

 private:
  CatalogItem(const char* path);

  // Construct from data read from catalog file
  CatalogItem(const char* path, time_t modTime, off_t sizeOriginal, std::vector<char>& vfile);

  std::string sImagePath;
  time_t imageModTime = 0;
  off_t imageSizeOriginal = 0;
  QByteArray iconBytes;
};

