#pragma once

#include <QMainWindow>
#include <QListView>
#include "SlideshowView.h"

class QMdiArea;
class QMdiSubWindow;
class Catalog;
class ImageView;
class ImageView;
class ListView;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();

  void openImageFile(const std::string& imageFilename);

private slots:
  void slotsubWindowActivated(QMdiSubWindow *window);

  void openImage();
  void printImage();
  void newCatalog();
  void openCatalog();
  void sortCatalog();
  void updateCatalog();
  void moveCatalogFiles();
  void trashFiles();
  void deleteCatalogFiles();
  void slideshowCatalog();

  void activateNextWindow();
  void activatePreviousWindow();
  void activatePreviousCatalog();
  void activateNextCatalog();
  void Settings();

  void slotslideshowkeyPress(void* slideshowId, SlideshowView::SlideshowAction result);
  void slotListViewCurrentChanged(const std::string& catalogFilename, const std::string& filename);

  void zoomIn();
  void zoomOut();
  void normalSize();
  void maximize();

  void togglefullscreen();

  void slideshowPrevious();
  void slideshowNext();
  void slideshowFirst();
  void slideshowLast();

  void copyImagePathSlash();
  void copyImagePathBackslash();

private:
  std::string getImagePath();

  enum class Direction { Next, Previous };
  void activateWindow(Direction n);

  template<typename T>
    T activeView();
  void CallListViewFunction(void (ListView::*listviewFunction)());

  template<int N>
    QMenu* populateMenu(const QString& menuName,
			struct menuItem (&items)[N]);
  std::vector<std::pair<QAction*, int> > enableAction;
  QMdiArea* mdiArea;
};

