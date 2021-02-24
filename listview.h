#include <QListView>
#include <QTime>
#include "SlideshowView.h"
//#include <QAbstractItemView>
class QModelIndex;
class MainWindow;
class Catalog;

class ListView : public QListView {
  Q_OBJECT
 public:
  ListView(MainWindow* mainwin, Catalog* cat);

  QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
  void keyReleaseEvent(QKeyEvent* e) override;
  void mouseDoubleClickEvent(QMouseEvent* e) override;
  void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

  Catalog* Cat() { return m_catalog; }

  void moveCatalogFiles();
  void trashCatalogFiles();
  void deleteCatalogFiles();
  void updateCatalog();
  void sortCatalog();

  SlideshowView* slideshowCatalog();
  std::string SlideshowkeyPress(SlideshowView* slideshow, SlideshowView::SlideshowAction result);

  //void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags);
  std::vector<QModelIndex> getSelections();

  std::string filename(int row = -1) const;

 signals:
  void signalCurrentChanged(const std::string& catalogFilename, const std::string& filename);

 private:
  void CallCatalogFunction(int (Catalog::*catalogFunction)(const std::vector<QModelIndex>& oldSelections));

  void OpenCurrentImage();
  MainWindow* const m_mainwin;
  Catalog* const m_catalog;

  QTime mostRecentSortTime;
};
