#include "catalog.h"
#include "catalogitem.h"
#include "listview.h"
#include "sortdialog.h"
#include "trashfiles.h"
#include "utility.h"
#include <QDirIterator>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <fstream>
//#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <algorithm>
 
FileTypes Catalog::imageFileTypes;

Catalog::Catalog(const char* folder, const char* catalogFile)
  : /*QStandardItemModel(0,0),*/ m_foldername(folder), m_catalogFileName(catalogFile) {
}

Catalog* Catalog::createEmpty() {
  QString filename, foldername;

  std::string catDir   = TivUtility::loadSetting("CatalogDir", (TivUtility::HomeDir() + "/Pictures").c_str()),
	      filesDir = TivUtility::loadSetting("CatalogFilesDir", (TivUtility::HomeDir() + "Pictures/").c_str());

  if ((filename =  QFileDialog::getSaveFileName(nullptr, tr("Create Catalog"),
						catDir.c_str(),
						tr("Catalog Files (*.tic)"))
       ).isEmpty()
      || (foldername = QFileDialog::getExistingDirectory(nullptr, "Catalog file folder", filesDir.c_str(),
							 QFileDialog::ShowDirsOnly | QFileDialog::DontConfirmOverwrite
							 | QFileDialog::ReadOnly | QFileDialog::DontUseCustomDirectoryIcons)
	  ).isEmpty())
    return nullptr;

  if (!filename.endsWith(".tic"))
    filename.append(".tic");

  TivUtility::saveSetting("CatalogDir", TivUtility::Dirname(filename.toUtf8().constData()).c_str());
  TivUtility::saveSetting("CatalogFilesDir", foldername.toUtf8().constData());


  Catalog* catItemModel = new Catalog(foldername.toUtf8().constData(), filename.toUtf8().constData());

  return catItemModel;
}

void Catalog::populateEmpty() {
  QDirIterator it(m_foldername.c_str(), Catalog::imageFileTypes,
		  QDir::Files | QDir::NoDotAndDotDot | QDir::Readable); // not QDir::CaseSensitive

  clock_t begin = std::clock();
  int n = 1;

  while (it.hasNext()) {
    it.next(); // After construction, the iterator is located before the first directory entry

    CatalogItem* item = CatalogItem::CreateFromImageFile(it.filePath().toUtf8().constData());
    if (item) {
      appendRow(item);
    }

    if (!(n++ % 100)) {
      emit signalCreatedItems();
    }
  }

  WriteNewCatalogFile();

  clock_t end = std::clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  printf("Elapsed time %f\n", elapsed_secs);
}

void Catalog::WriteHeader(std::ofstream& ofsCatalogFile) const {
  ofsCatalogFile << "1 0 1 " /* version major, minor, #folders */
		 << m_foldername.size() << " " << m_foldername.c_str() << std::endl;
}
void Catalog::WriteNewCatalogFile() const { // Write new catalog file
  std::ofstream ofsCatalogFile(m_catalogFileName, std::ofstream::trunc | std::ofstream::binary);

  WriteHeader(ofsCatalogFile);

  QStandardItem* item;
  CatalogItem* catitem;
  for (int row=0; row<rowCount(); ++row) {
    QModelIndex modelIndex = index(row, 0, QModelIndex());
    if ((item = itemFromIndex(modelIndex)) != nullptr
	&& (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {
      catitem->WriteToCatalog(ofsCatalogFile);
    }
  }
}

Catalog* Catalog::open() {
  std::string catDir = TivUtility::loadSetting("CatalogDir", (TivUtility::HomeDir() + "/Pictures").c_str());
  QString catalogFilename(QFileDialog::getOpenFileName(nullptr, tr("Open Catalog"),
						       catDir.c_str(),
						       tr("Catalog Files (*.tic)")));
  if (catalogFilename.isEmpty())
    return nullptr;

  TivUtility::saveSetting("CatalogDir", TivUtility::Dirname(catalogFilename.toUtf8().constData()).c_str());

  std::ifstream ifsCatalogFile(catalogFilename.toUtf8().constData(), std::ifstream::binary);

  int versionMajor, versionMinor,
      foldercount, foldernamelength;
  char space, endl;

  ifsCatalogFile >> std::noskipws
		 >> versionMajor >> space >> versionMinor >> space
		 >> foldercount >> space >> foldernamelength >> space;

  if (foldercount != 1)
    return nullptr; // >1 folders not supported in version 1

  std::vector<char> foldername(foldernamelength+1);
  ifsCatalogFile.read(&foldername[0], foldernamelength);
  foldername[foldernamelength] = '\0';
  ifsCatalogFile >> endl;

  Catalog* catItemModel = new Catalog(&foldername[0], catalogFilename.toUtf8().constData());

  CatalogItem* item;
  while ((item = CatalogItem::CreateFromCatalog(ifsCatalogFile)) != nullptr) {
    catItemModel->appendRow(item);
  }

  return catItemModel;
}

// Return lambda function to use for sorting CatalogItem s depending on sort dialog choices
std::function<bool (const CatalogItem* lhs, const CatalogItem* rhs)>
Catalog::sortFunction(const std::string& sortBy, bool ascending) {
  if (sortBy.compare("Path") == 0 && ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ImagePath().compare(rhs->ImagePath()) < 0; };
  }
  if (sortBy.compare("Path") == 0 && !ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ImagePath().compare(rhs->ImagePath()) > 0; };
  }
  if (sortBy.compare("Date") == 0 && ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ModTime() < rhs->ModTime(); };
  }
  if (sortBy.compare("Date") == 0 && !ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ModTime() > rhs->ModTime(); };
  }
  if (sortBy.compare("Size") == 0 && ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ImageSize() < rhs->ImageSize(); };
  }
  if (sortBy.compare("Size") == 0 && !ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ImageSize() > rhs->ImageSize(); };
  }
  if (sortBy.compare("Name") == 0 && ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ImageBasename().compare(rhs->ImageBasename()) < 0; };
  }
  if (sortBy.compare("Name") == 0 && !ascending) {
    return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
      return lhs->ImageBasename().compare(rhs->ImageBasename()) > 0; };
  }

  // Else sort by Date descending. Should never happen
  return [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
    return lhs->ModTime() > rhs->ModTime(); };
}

// Another way to sort: https://doc.qt.io/qt-5/qsortfilterproxymodel.html
std::vector<QModelIndex> Catalog::sort(const std::vector<QModelIndex>& oldSelections) {
  std::vector<QModelIndex> rv; // New selections
  const int nRows = rowCount();
  if (nRows < 2) return rv; // Nothing to sort

  SortDialog sortDlg;
  if (sortDlg.exec() != QDialog::Accepted)
    return rv; // User changed mind

  std::vector<CatalogItem*> vItems;

  QStandardItem* standarditem;
  CatalogItem* catitem;
  for (int row=0; row<nRows; ++row) {
    QModelIndex modelindex = index(row, 0);
    if ((standarditem = itemFromIndex(modelindex)) != nullptr
	&& (catitem = static_cast<CatalogItem*>(standarditem)) != nullptr) {

      catitem->bCurrent = catitem->bSelected = false;
      vItems.push_back(catitem);
    }
  }

  if (!oldSelections.empty() && (int)vItems.size() > oldSelections[0].row()) {
    vItems[oldSelections[0].row()]->bCurrent = true;
  }
  for (unsigned int i=1; i<oldSelections.size(); ++i) {
    if ((int)vItems.size() > oldSelections[i].row()) {
      vItems[oldSelections[i].row()]->bSelected = true;
    }
  }


  std::sort(vItems.begin(), vItems.end(), sortFunction(sortDlg.sortField(), sortDlg.isAscending()));


  for (int n; (n=rowCount())>0; takeRow(n-1)) // Clear catalog without deleting catalogitems
    ;

  // Insert catalog items and preserve current & selected items into rv return value
  std::vector<int> selected;
  for (CatalogItem* catitem : vItems) {
    appendRow(catitem);
    if (catitem->bCurrent) {
      QModelIndex currentModelIndex = index(rowCount()-1, 0);
      rv.push_back(currentModelIndex);
    }
    if (catitem->bSelected)
      selected.push_back(rowCount()-1);
  }
  Q_ASSERT(rv.size() == 1);
  if (rv.empty()) // No current item; should never happen
    rv.push_back(QModelIndex());

  for (int s : selected) {
    QModelIndex selectedModelIndex = index(s, 0);
    rv.push_back(selectedModelIndex);
  }

  WriteNewCatalogFile(); // Write new catalog file in sorted order
  return rv;
}

int Catalog::update(const QModelIndex& current) {
  // Prepare to preserve current item selection
  const int currentRowNumber = current.row();
  int nDeletedBeforeCurrent = 0;
  bool deletedCurrent = false;

  // Remove any deleted files, Update changed files
  std::vector<CatalogItem*> sortedItems;
  QStandardItem* item;
  CatalogItem* catitem;
  // In descending order so later deletions will be correct
  for (int row=rowCount()-1; row>=0; --row) {
    QModelIndex modelIndex = index(row, 0, QModelIndex());
    if ((item = itemFromIndex(modelIndex)) != nullptr
	&& (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {
      bool deletedFile = access(catitem->ImagePath().c_str(), R_OK) != 0;
      if (deletedFile) {
	QList<QStandardItem*> itemlist = takeRow(row);
	delete catitem;	

	if (currentRowNumber == row)
	  deletedCurrent = true;
	if (row < currentRowNumber)
	  ++nDeletedBeforeCurrent;
      } else {
	if (catitem->Changed())
	  catitem->UpdateFromFile();
	sortedItems.push_back(catitem);
      }
    }
  }

  // Add any new files to end
  auto SortByName = [](const CatalogItem* lhs, const CatalogItem* rhs) -> bool {
    return lhs->ImagePath().compare(rhs->ImagePath()) < 0;
  };
  auto FindByName = [](const CatalogItem* lhs, const char* rhs) -> bool {
    return lhs->ImagePath().compare(rhs) < 0;
  };
  std::sort(sortedItems.begin(), sortedItems.end(), SortByName);
  QDirIterator itFiles(m_foldername.c_str(), Catalog::imageFileTypes,
		       QDir::Files | QDir::NoDotAndDotDot | QDir::Readable); // not QDir::CaseSensitive
  while (itFiles.hasNext()) {
    itFiles.next(); // After construction, the iterator is located before the first directory entry

    const std::string sFilename = itFiles.filePath().toUtf8().constData();
    const char* filename = sFilename.c_str();
    auto itItem = std::lower_bound(sortedItems.cbegin(), sortedItems.cend(), filename, FindByName);

    if ((itItem == sortedItems.end() || (*itItem)->ImagePath().compare(filename) != 0)
	&& (catitem = CatalogItem::CreateFromImageFile(filename)) != nullptr)
      appendRow(catitem);
  }

  WriteNewCatalogFile();

  // Compute row number for caller to set as current item
  const int nRows = rowCount(),
            newRowNumber = currentRowNumber - nDeletedBeforeCurrent;

  return !deletedCurrent ? -1               // Current is still selected so no action needed
       : (nRows == 0) ? -1                  // Empty catalog
       : (newRowNumber < 0) ? 0             // Before first -> first
       : (newRowNumber > nRows-1) ? nRows-1 // After last -> last
       : newRowNumber;                      // OK
}

Catalog::MoveResult Catalog::MoveFile(const char* targetDir, const char* sourcePath) {
  if (access(sourcePath, R_OK) != 0)
    return MoveResult::ERROR; // Source missing

  std::string originalFilename = TivUtility::Basename(sourcePath),
              targetPath = std::string(targetDir) + "/" + originalFilename;

  if (!strcmp(sourcePath, targetPath.c_str()))
    return MoveResult::SAME_DIR; // Can't move file to same folder

  unlink(targetPath.c_str());
  if (access(targetPath.c_str(), R_OK) == 0)
    return MoveResult::ERROR; // Target exists but couldn't be deleted

  if (rename(sourcePath, targetPath.c_str()) == 0)
    return MoveResult::SUCCESS;

  // Error renaming file so copy and delete original
  std::ifstream  src(sourcePath, std::ifstream::binary);
  std::ofstream  dst(targetPath, std::ofstream::binary);
  dst << src.rdbuf();

  if (access(targetPath.c_str(), R_OK) != 0)
    return MoveResult::ERROR; // File not copied

  unlink(sourcePath); // Delete original

  return MoveResult::SUCCESS;
}

int Catalog::moveFiles(const std::vector<QModelIndex>& oldSelections) {
  const int nSelected = oldSelections.size();
  if (nSelected < 2)
    return -1; // An item is current but nothing selected to move

  QString title = QString::asprintf("Move %d file%s to folder", nSelected-1, nSelected>2 ? "s" : "");
  QString foldername = QFileDialog::getExistingDirectory(nullptr, title, (TivUtility::HomeDir() + "Pictures/").c_str(),
							 QFileDialog::ShowDirsOnly | QFileDialog::DontConfirmOverwrite
							 | QFileDialog::ReadOnly | QFileDialog::DontUseCustomDirectoryIcons);
  if (foldername.isEmpty())
    return -1;

  const int currentRowNumber = oldSelections.front().row();
  int nDeletedBeforeCurrent = 0;

  // Sort in descending order so later deletions will be correct
  std::vector<QModelIndex> sortedIndices(++oldSelections.cbegin(), oldSelections.cend());
  std::sort(sortedIndices.begin(), sortedIndices.end(),
	    [](const QModelIndex& lhs, const QModelIndex& rhs) {
	      return lhs.row() > rhs.row(); } );
  
  QStandardItem* item;
  CatalogItem* catitem;
  for (const QModelIndex& ix : sortedIndices) {
    if ((item = itemFromIndex(ix)) != nullptr
	&& (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {

      std::string sOriginalFilePath = catitem->ImagePath();
      if (MoveFile(foldername.toUtf8().constData(), sOriginalFilePath.c_str()) == MoveResult::SUCCESS) {      
	int thisRow = ix.row();//catitem->row();
	if (thisRow < currentRowNumber)
	  ++nDeletedBeforeCurrent;

	QList<QStandardItem*> itemlist = takeRow(thisRow);
	delete catitem;
      }
    }
  }

  WriteNewCatalogFile(); // Write new catalog file with deleted items removed

  // Compute row number for caller to set as current item
  const int nRows = rowCount(),
            newRowNumber = currentRowNumber - nDeletedBeforeCurrent;

  return (nRows == 0) ? -1                  // Empty catalog
       : (newRowNumber < 0) ? 0             // Before first -> first
       : (newRowNumber > nRows-1) ? nRows-1 // After last -> last
       : newRowNumber;                      // OK
}

int Catalog::trashFiles(const std::vector<QModelIndex>& oldSelections) {
  const int nSelected = oldSelections.size();
  if (nSelected < 2)
    return -1; // An item is current but nothing selected for delete

  TrashFiles tf;
  if (!tf.isOk()) {
    QMessageBox::question(nullptr, "Missing directory",
			  "Can't find the trash directory, nothing trashed.",
			  QMessageBox::Ok);
    return -1;
  }

  const int currentRowNumber = oldSelections.front().row();
  int nDeletedBeforeCurrent = 0;

  // Sort in descending order so later deletions will be correct
  std::vector<QModelIndex> sortedIndices(++oldSelections.cbegin(), oldSelections.cend());
  std::sort(sortedIndices.begin(), sortedIndices.end(),
	    [](const QModelIndex& lhs, const QModelIndex& rhs) {
	      return lhs.row() > rhs.row(); } );

  QStandardItem* item;
  CatalogItem* catitem;
  for (const QModelIndex& ix : sortedIndices) {
    if ((item = itemFromIndex(ix)) != nullptr
	&& (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {
      //std::string sOriginalFilePath = catitem->ImagePath();

      //using TrashFiles::TrashResult;
      const TrashFiles::TrashResult result = tf.TrashOneFile(catitem->ImagePath().c_str());
      if (result == TrashFiles::TrashResult::SUCCESS
       || result == TrashFiles::TrashResult::MISSING_FILE) {
	int thisRow = ix.row();//catitem->row();
	if (thisRow < currentRowNumber)
	  ++nDeletedBeforeCurrent;

	QList<QStandardItem*> itemlist = takeRow(thisRow);
	delete catitem;
      }
    }
  }

  // TODO: Inform user about trashfile errors

  WriteNewCatalogFile(); // Write new catalog file with deleted items removed

  // Compute row number for caller to set as current item
  const int nRows = rowCount(),
            newRowNumber = currentRowNumber - nDeletedBeforeCurrent;

  return (nRows == 0) ? -1                  // Empty catalog
       : (newRowNumber < 0) ? 0             // Before first -> first
       : (newRowNumber > nRows-1) ? nRows-1 // After last -> last
       : newRowNumber;                      // OK
}

int Catalog::deleteFiles(const std::vector<QModelIndex>& oldSelections) {
  const int nSelected = oldSelections.size();
  if (nSelected < 2)
    return -1; // An item is current but nothing selected for delete

  QString ask = QString::asprintf("Permanently delete %d file(s)?", nSelected-1);
  QMessageBox::StandardButton reply
    = QMessageBox::question(nullptr, "Delete", ask, QMessageBox::Yes|QMessageBox::No);
  if (reply != QMessageBox::Yes)
    return -1;
 
  const int currentRowNumber = oldSelections.front().row();
  int nDeletedBeforeCurrent = 0;

  // Sort in descending order so later deletions will be correct
  std::vector<QModelIndex> sortedIndices(++oldSelections.cbegin(), oldSelections.cend());
  std::sort(sortedIndices.begin(), sortedIndices.end(),
	    [](const QModelIndex& lhs, const QModelIndex& rhs) {
	      return lhs.row() > rhs.row(); } );

  QStandardItem* item;
  CatalogItem* catitem;
  for (const QModelIndex& ix : sortedIndices) {
    if ((item = itemFromIndex(ix)) != nullptr
	&& (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {

      unlink(catitem->ImagePath().c_str()); // User OK to permanently delete file

      int thisRow = ix.row();//catitem->row();
      if (thisRow < currentRowNumber)
	++nDeletedBeforeCurrent;

      QList<QStandardItem*> itemlist = takeRow(thisRow);
      delete catitem;
    }
  }

  WriteNewCatalogFile(); // Write new catalog file with deleted items removed

  // Compute row number for caller to set as current item
  const int nRows = rowCount(),
            newRowNumber = currentRowNumber - nDeletedBeforeCurrent;

  return (nRows == 0) ? -1                  // Empty catalog
       : (newRowNumber < 0) ? 0             // Before first -> first
       : (newRowNumber > nRows-1) ? nRows-1 // After last -> last
       : newRowNumber;                      // OK
}

std::string Catalog::filename(int row) {
  QStandardItem* item;
  CatalogItem* catitem;
  if (0 <= row && row < rowCount()) {
    QModelIndex modelIndex = index(row, 0, QModelIndex());
    if ((item = itemFromIndex(modelIndex)) != nullptr
	&& (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {
      return catitem->ImagePath();
    }
  }
  return "";
}

