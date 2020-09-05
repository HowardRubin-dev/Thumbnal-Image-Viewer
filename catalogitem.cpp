#include <QBuffer>
#include <fstream>
#include <sys/stat.h>
#include "catalogitem.h"
#include "utility.h"

CatalogItem::CatalogItem(const char* path) : sImagePath(path) {
}

// Construct from catalog file data
CatalogItem::CatalogItem(const char* path, time_t modTime, off_t sizeOriginal, std::vector<char>& vfile) : QStandardItem(),
    sImagePath(path), imageModTime(modTime), imageSizeOriginal(sizeOriginal),
    iconBytes(&vfile[0], vfile.size())
{
  QPixmap pixmap;
  pixmap.loadFromData(iconBytes, "JPG");
  QIcon ItemIcon(pixmap);
  setIcon(ItemIcon);
}

CatalogItem* CatalogItem::CreateFromImageFile(const char* imagePath) {
  CatalogItem* item = new CatalogItem(imagePath);
  if (!item->UpdateFromFile()) {
    delete item;
    return nullptr;
  } else
    return item;
  return nullptr;
}

CatalogItem* CatalogItem::CreateFromCatalog(std::ifstream& ifsCatalogFile) {
  int filenameLength=0, imageSizeReduced;
  char space, endl;

  if (!(ifsCatalogFile >> filenameLength >> space) || filenameLength <= 0)
    return nullptr;

  std::vector<char> filename(filenameLength+1);
  ifsCatalogFile.read(&filename[0], filenameLength);
  filename[filenameLength] = '\0';

  off_t imageSizeOriginal;
  time_t imageModTime;
  ifsCatalogFile >> space
		 >> imageModTime >> space
		 >> imageSizeOriginal >> space
		 >> imageSizeReduced >> space;
  std::vector<char> imageReduced(imageSizeReduced);
  ifsCatalogFile.read(&imageReduced[0], imageSizeReduced);
  ifsCatalogFile >> endl;

  CatalogItem* item = new CatalogItem(&filename[0], imageModTime, imageSizeOriginal, imageReduced);
  return item;
}

bool CatalogItem::UpdateFromFile() {
  struct stat st;
  if (stat(sImagePath.c_str(), &st) != 0) {
    Q_ASSERT_X(false, "CatalogItem::UpdateFromFile()", "File must exist");
    return false;
  }
  QPixmap p1(sImagePath.c_str());
  if (p1.isNull())
    return false;

  imageModTime = st.st_mtime;
  imageSizeOriginal = st.st_size;

  QPixmap p2 = p1.scaled(128, 128, Qt::KeepAspectRatio, Qt::FastTransformation);

  QBuffer buffer(&iconBytes);
  buffer.open(QIODevice::WriteOnly);
  p2.save(&buffer, "JPG"); // write pixmap into bytes

  QIcon ItemIcon(p2);
  setIcon(ItemIcon);
  //setIconSize(QSize(128, 128));//pixmap.rect().size());

  return true;
}

/*bool CatalogItem::Changed(const struct stat& st) const {
  return st.st_mtime != imageModTime || st.st_size != imageSizeOriginal;
  }*/
// File must exist
bool CatalogItem::Changed() const {
  struct stat st;
  if (stat(sImagePath.c_str(), &st) != 0) {
    Q_ASSERT_X(false, "CatalogItem::Changed()", "File must exist");
    return false;
  }
  return st.st_mtime != imageModTime || st.st_size != imageSizeOriginal;
}

void CatalogItem::WriteToCatalog(std::ofstream& ofsCatalogFile) const {
  ofsCatalogFile << strlen(sImagePath.c_str()) << " " << sImagePath.c_str() << " " // filename
		 << imageModTime << " " << imageSizeOriginal << " " // file size and modification time
		 << iconBytes.size() << " "; // reduced image size

  ofsCatalogFile.write(iconBytes.constData(), iconBytes.size()); // 128x128 image data
  ofsCatalogFile << std::endl;
}

const std::string CatalogItem::ImageBasename() const {
  return TivUtility::Basename(sImagePath);
}
