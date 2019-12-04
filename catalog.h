#pragma once

#include <QStandardItemModel>
class ListView;
class MainWindow;

class FileTypes {
 public:
  FileTypes() { list << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp"; }
  operator const QStringList&() const { return list; }
 private:
  QStringList list;
};

class Catalog : public QStandardItemModel {
  Q_OBJECT // Enable signal signalCreatedItems()
 public:

  static Catalog* createEmpty();
  void populateEmpty();

  static Catalog* open();
  void setListView(ListView* l) { m_listview = l; }
  std::vector<QModelIndex> sort(const std::vector<QModelIndex>& oldSelections);
  int update(const QModelIndex& current);

  int moveFiles(const std::vector<QModelIndex>& oldSelections); // Returns new current item row number
  int trashFiles(const std::vector<QModelIndex>& oldSelections); // Returns new current item row number
  int deleteFiles(const std::vector<QModelIndex>& oldSelections); // Returns new current item row number

  const std::string& CatalogFileName() { return m_catalogFileName; }

  enum class MoveResult { SUCCESS, ERROR, SAME_DIR, };
  static MoveResult MoveFile(const char* targetDir, const char* sourcePath);

  std::string filename(int row);

 signals:
  void signalCreatedItems();

 private:
  Catalog(const char* folder, const char* catalogFile);

  void WriteHeader(std::ofstream& ofsCatalogFile) const;
  void WriteNewCatalogFile() const; // Write new catalog file
  const std::string m_foldername;
  const std::string m_catalogFileName;
  ListView* m_listview = nullptr;
  static FileTypes imageFileTypes;
};
