#include "mainwindow.h"
#include "catalog.h"
#include "catalogitem.h"
#include "ImageView.h"
#include "listview.h"
#include "utility.h"
#include "SlideshowView.h"
#include "menuitem.h"

#include <QKeyEvent>
#include <QListView>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMessageBox>

template<int N>
QMenu* MainWindow::populateMenu(const QString& menuName, struct menuItem (&items)[N]) {
  QMenu* menu = menuBar()->addMenu(menuName);
  for (struct menuItem item : items) {
    if (!item.itemName.compare("Separator")) {
      menu->addSeparator();
    } else if (!item.itemName.isEmpty() && item.receiver && item.method) {
      QAction* action = new QAction(item.itemName, this);
      connect(action, SIGNAL(triggered()), item.receiver, item.method);
      if (!item.keys.isEmpty() && !item.keys.first().isEmpty())
	action->setShortcuts(item.keys);
      menu->addAction(action);

      enableAction.emplace_back(action, item.enableMenu); // Prepare to Update menus on subwindow change
    }
  }
  return menu;
}
////////////////////////////////////////////////////////////////

MainWindow::MainWindow() {
  setGeometry(0,0,1500,1200);
  mdiArea = new QMdiArea;
  setCentralWidget(mdiArea);

  //QMdiArea::WindowOrder order = mdiArea->activationOrder();
  //printf("WindowOrder = %d\n", order);
  //mdiArea->setActivationOrder

  struct menuItem fileMenuItems[] = {
    { tr("&Open Image..."),   this, SLOT(openImage()),          QKeySequence::Open, ALWAYS },
    { tr("&Print Image..."),  this, SLOT(printImage()),         QKeySequence(tr("CTRL+p")), IMAGE|SLIDESHOW, },
    { tr("&Trash file(s)"),   this, SLOT(trashFiles()),         QKeySequence(tr("DELETE")), ANY },
    { tr("&Delete Files..."), this, SLOT(deleteCatalogFiles()), QKeySequence(tr("SHIFT+DELETE")), CATALOG },
    { tr("Close Program"),    this, SLOT(close()),              QKeySequence(tr("ALT+F4")), ALWAYS },
  };
  populateMenu(tr("&File"), fileMenuItems);

  struct menuItem viewMenuItems[] = {
    { tr("Next Window"),       mdiArea, SLOT(activateNextSubWindow()),
      QList<QKeySequence> { QKeySequence::NextChild,
			    QKeySequence(Qt::CTRL+Qt::Key_F6) }, ANY2ORMORE },
    { tr("Previous Window"),   this, SLOT(activatePreviousWindow()),
      QList<QKeySequence> { //QKeySequence::PreviousChild, // Doesn't work
			    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Tab),
			    QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_F6) }, ANY2ORMORE },
    { tr("Next Catalog"),        this, SLOT(activateNextCatalog()),     QKeySequence(tr("CTRL+c")), IMAGE|CATALOG },
    { tr("Previous Catalog"),    this, SLOT(activatePreviousCatalog()), QKeySequence(tr("CTRL+SHIFT+c")), IMAGE|CATALOG },

    { tr("Ma&ximize"),           this, SLOT(maximize()),          QKeySequence(tr("x")), ANY },
    { tr("Normal Si&ze"),        this, SLOT(normalSize()),        QKeySequence(tr("z")), ANY },
    { tr("Zoom &In (25%)"),      this, SLOT(zoomIn()),            QKeySequence::ZoomIn, IMAGE|SLIDESHOW },
    { tr("Zoom &Out (25%)"),     this, SLOT(zoomOut()),           QKeySequence::ZoomOut, IMAGE|SLIDESHOW },
    { "Separator" },
    { tr("Toggle &Full screen"), this, SLOT(togglefullscreen()),  QKeySequence(tr("f")), ALWAYS },
    { tr("Cascade Windows"),  mdiArea, SLOT(cascadeSubWindows()), QKeySequence(), ANY2ORMORE },
    { tr("Tile Windows"),     mdiArea, SLOT(tileSubWindows()),    QKeySequence(), ANY2ORMORE },
  };
  populateMenu(tr("&View"), viewMenuItems);

  struct menuItem catalogMenuItems[] = {
    { tr("&New Catalog..."),  this, SLOT(newCatalog()),       QKeySequence(), ALWAYS },
    { tr("&Open Catalog..."), this, SLOT(openCatalog()),      QKeySequence(tr("CTRL+g")), ALWAYS },
    { tr("&Sort..."),         this, SLOT(sortCatalog()),      QKeySequence(), CATALOG },
    { tr("&Update"),          this, SLOT(updateCatalog()),    QKeySequence(tr("SHIFT+u")), CATALOG },
    { tr("&Move file(s)..."), this, SLOT(moveCatalogFiles()), QKeySequence(), CATALOG },
    { tr("SlideSho&w"),       this, SLOT(slideshowCatalog()), QKeySequence(), CATALOG },
    // Scan Folder, Properties, Record properties, Find record
  };
  populateMenu(tr("&Catalog"), catalogMenuItems);

  struct menuItem SlideshowMenuItems[] = {
    { tr("Previous slide"), this, SLOT(slideshowPrevious()), QKeySequence(tr("left")),  SLIDESHOW },
    { tr("Next slide"),     this, SLOT(slideshowNext()),     QKeySequence(tr("right")), SLIDESHOW },
    { tr("First slide"),    this, SLOT(slideshowFirst()),    QKeySequence(tr("home")),  SLIDESHOW },
    { tr("Last slide"),     this, SLOT(slideshowLast()),     QKeySequence(tr("end")),   SLIDESHOW },
  };
  populateMenu(tr("&Slideshow"), SlideshowMenuItems);

  slotsubWindowActivated(nullptr); // Update menu. Could be done in earlier initialization.
  connect(mdiArea, &QMdiArea::subWindowActivated, // Update menus on each subwindow change.
	  this, &MainWindow::slotsubWindowActivated);
}

void MainWindow::slotsubWindowActivated(QMdiSubWindow* subWindow) {

  ImageView* image         = nullptr;
  ListView* list           = nullptr;
  SlideshowView* slideshow = nullptr;

  if (subWindow != nullptr) {
    image     = qobject_cast<ImageView*>(subWindow->widget());
    list      = dynamic_cast<ListView*>(subWindow->widget());
    slideshow = qobject_cast<SlideshowView*>(subWindow->widget());
  }

  bool visible = false;
  for (const auto& p : enableAction) {
    switch (p.second) {
    case ALWAYS:
      visible = true; break;
    case ANY2ORMORE:
      visible = mdiArea->subWindowList().size() >= 2; break;
    case IMAGE:
      visible = image != nullptr; break;
    case CATALOG:
      visible = list != nullptr; break;
    case SLIDESHOW:
      visible = slideshow != nullptr; break;
    case IMAGE | SLIDESHOW:
      visible = image || slideshow; break;
    case IMAGE | CATALOG:
      visible = image || list; break;
    case ANY:
      visible = image || list || slideshow; break;
    default:
    case NEVER:
      printf("case NEVER in MainWindow::slotsubWindowActivated()\n");
      Q_ASSERT_X(false, "MainWindow::slotsubWindowActivated()", "Unsupported enum");
      break;
    }

    p.first->setVisible(visible);
    //p.first->setEnabled(enabled);
  }
}

template<typename T>
T MainWindow::activeView() {
  QMdiSubWindow* subWindow = mdiArea->activeSubWindow();
  if (subWindow)
    return qobject_cast<T>(subWindow->widget());
  else
    return nullptr;
}

void MainWindow::printImage() {
  ImageView* image;
  SlideshowView* slideshow;
  if ((image = activeView<ImageView*>()) != nullptr)
    image->Print();
  else if ((slideshow = activeView<SlideshowView*>()) != nullptr)
    slideshow->UserAction(SlideshowView::SlideshowAction::PRINT);
}

void MainWindow::openImageFile(const std::string& imageFilename) {
  ImageView* view;
  QWidget* widget;

  // Activate any existing image window
  QList<QMdiSubWindow*> subwindows = mdiArea->subWindowList();
  for (auto it=subwindows.cbegin(); it!=subwindows.cend(); ++it) {
    if ((widget = (*it)->widget()) != nullptr
	&& (view = qobject_cast<ImageView*>(widget)) != nullptr
	&& imageFilename.compare(view->filename()) == 0) {

      widget->setFocus();
      return;
    }
  }

  // Otherwise open new image window
  QMdiSubWindow *subWindow;
  if ((view = ImageView::openFile(imageFilename.c_str())) != nullptr
      && (subWindow = mdiArea->addSubWindow(view)) != nullptr) {
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowTitle(TivUtility::Basename(view->filename()).c_str());
    subWindow->show();
  }
}

void MainWindow::openImage() {
  ImageView* view;
  QMdiSubWindow *subWindow;

  if ((view = ImageView::open()) != nullptr
      && (subWindow = mdiArea->addSubWindow(view)) != nullptr) {
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowTitle(TivUtility::Basename(view->filename()).c_str());
    subWindow->show();
  }
}

/*void MainWindow::CallOneOrTwoFunctions(void (ImageView::*viewFunction)(),
				       void (QMdiSubWindow::*subWindowFunction)()) {
  ImageView* view = activeView<ImageView*>();
  QMdiSubWindow* subWindow;
  if (view)
    (view->*viewFunction)();
  else if (subWindowFunction
	   && (subWindow = mdiArea->activeSubWindow()) != nullptr)
    (subWindow->*subWindowFunction)();
    }*/

void MainWindow::zoomIn() {
  ImageView* view;
  SlideshowView* slideshow;
  if ((view = activeView<ImageView*>()) != nullptr)
    view->zoomIn();
  else if ((slideshow = activeView<SlideshowView*>()) != nullptr)
    slideshow->UserAction(SlideshowView::SlideshowAction::ZOOMIN);
}
void MainWindow::zoomOut() {
  ImageView* view;
  SlideshowView* slideshow;
  if ((view = activeView<ImageView*>()) != nullptr)
    view->zoomOut();
  else if ((slideshow = activeView<SlideshowView*>()) != nullptr)
    slideshow->UserAction(SlideshowView::SlideshowAction::ZOOMOUT);
}
void MainWindow::normalSize() {
  ImageView* view;
  SlideshowView* slideshow;
  QMdiSubWindow* subWindow;
  if ((view = activeView<ImageView*>()) != nullptr)
    view->normalSize();
  else if ((slideshow = activeView<SlideshowView*>()) != nullptr)
    slideshow->UserAction(SlideshowView::SlideshowAction::NORMALSIZE);
  else if ((subWindow = mdiArea->activeSubWindow()) != nullptr)
    subWindow->showNormal();
}
void MainWindow::maximize() {
  ImageView* image;
  SlideshowView* slideshow;
  QMdiSubWindow* subWindow;
  if ((image = activeView<ImageView*>()) != nullptr)
    image->maximize();
  else if ((slideshow = activeView<SlideshowView*>()) != nullptr)
    slideshow->UserAction(SlideshowView::SlideshowAction::MAXIMIZE);
  else if ((subWindow = mdiArea->activeSubWindow()) != nullptr)
    subWindow->showMaximized();
}

void MainWindow::togglefullscreen() {
  if (isFullScreen())
    showNormal(); // Show main window normal size
  else 
    showFullScreen(); // Show main window full screen
}

// QMdiArea::activatePreviousSubWindow() doesn't work as advertised
// See bug report https://bugreports.qt.io/browse/QTBUG-22526
void MainWindow::activatePreviousWindow() {
  const QList<QMdiSubWindow*> subwindows = mdiArea->subWindowList();
  QMdiSubWindow* activeSubWin;
  QList<QMdiSubWindow*>::const_iterator it;
  const auto cbegin = subwindows.cbegin(),
             cend   = subwindows.cend();

  if (!subwindows.empty() // Sanity checks
      && (activeSubWin = mdiArea->activeSubWindow()) != nullptr
      && (it = std::find(cbegin, cend, activeSubWin)) != cend) {

    if (it == cbegin) // Active window is first
      it = cend; // So wrap around to end before decrementing iterator
    mdiArea->setActiveSubWindow(*--it);
  }
}

static bool isCatalog(const QMdiSubWindow* it) {
  return dynamic_cast<ListView*>(it->widget()) != nullptr;
}
void MainWindow::activateNextCatalog() {
  const QList<QMdiSubWindow*> subwindows = mdiArea->subWindowList();
  QMdiSubWindow* activeSubWin;
  QList<QMdiSubWindow*>::const_iterator itActive, itCatalog;
  const auto cbegin = subwindows.cbegin(),
             cend   = subwindows.cend();

  if (!subwindows.empty() // Sanity checks
      && (activeSubWin = mdiArea->activeSubWindow()) != nullptr
      && (itActive = std::find(cbegin, cend, activeSubWin)) != cend) {

    if ((itCatalog = std::find_if(itActive+1, cend, isCatalog)) != cend)
      mdiArea->setActiveSubWindow(*itCatalog); // First catalog after active window
    else if ((itCatalog = std::find_if(cbegin, itActive, isCatalog)) != cend)
      mdiArea->setActiveSubWindow(*itCatalog); // First catalog before active window
  }
}
void MainWindow::activatePreviousCatalog() {
  const QList<QMdiSubWindow*> subwindows = mdiArea->subWindowList();
  QMdiSubWindow* activeSubWin;
  QList<QMdiSubWindow*>::const_reverse_iterator itActive, itCatalog;
  const auto crbegin = subwindows.crbegin(),
             crend   = subwindows.crend();

  if (!subwindows.empty() // Sanity checks
      && (activeSubWin = mdiArea->activeSubWindow()) != nullptr
      && (itActive = std::find(crbegin, crend, activeSubWin)) != crend) {

    if ((itCatalog = std::find_if(itActive+1, crend, isCatalog)) != crend)
      mdiArea->setActiveSubWindow(*itCatalog); // Last catalog before active window
    else if ((itCatalog = std::find_if(crbegin, itActive, isCatalog)) != crend)
      mdiArea->setActiveSubWindow(*itCatalog); // Last catalog after active window
  }
}

void MainWindow::openCatalog() {
  Catalog* cat;
  QMdiSubWindow* subWindow; // QMdiSubWindow->repaint()
  ListView* listview;
  TivUtility::WaitCursor<MainWindow> wc(this);

  if ((cat = Catalog::open()) != nullptr
      && (listview = new ListView(this, cat)) != nullptr
      && (subWindow = mdiArea->addSubWindow(listview)) != nullptr) {
    cat->setListView(listview);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowTitle(cat->CatalogFileName().c_str());
    subWindow->showMaximized();

    connect(listview, &ListView::signalCurrentChanged,
	    this, &MainWindow::slotListViewCurrentChanged);

    QModelIndex ix = cat->index(0, 0);
    listview->setCurrentIndex(ix);
  }
}

void MainWindow::newCatalog() {
  Catalog* cat = nullptr;
  ListView* listview = nullptr;
  QMdiSubWindow* subWindow = nullptr;

  if ((cat = Catalog::createEmpty()) == nullptr
      || (listview = new ListView(this, cat)) == nullptr
      || (subWindow = mdiArea->addSubWindow(listview)) == nullptr) {
    //delete subWindow; // Always nullptr here
    delete listview;
    delete cat;
    // TODO: think about object lifetimes and notify user
  } else {

    cat->setListView(listview);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->showMaximized();

    TivUtility::WaitCursor<MainWindow> wc(this);

    // From https://wiki.qt.io/New_Signal_Slot_Syntax
    // Called once every 100 images added to the catalog to inform user how things are moving
    auto slotCreatedItems =
      [&]() {
	const int nRows = cat->rowCount();
	/*QModelIndex ix = cat->index(nRows-1, 0);
	listview->setCurrentIndex(ix);
	listview->repaint();*/

	subWindow->setWindowTitle(QString::asprintf("%d", nRows));
    };

    QMetaObject::Connection conn = connect(cat, &Catalog::signalCreatedItems,
					   this, slotCreatedItems);
    cat->populateEmpty();
    disconnect(conn);

    connect(listview, &ListView::signalCurrentChanged,
	    this, &MainWindow::slotListViewCurrentChanged);

    subWindow->setWindowTitle(cat->CatalogFileName().c_str());

    QModelIndex ix = cat->index(0, 0);
    listview->setCurrentIndex(ix);
  }
}

void MainWindow::CallListViewFunction(void (ListView::*listviewFunction)()) {
  TivUtility::WaitCursor<MainWindow> wc(this);
  ListView* list;
  if ((list = activeView<ListView*>()) != nullptr)
    (list->*listviewFunction)();
}
void MainWindow::sortCatalog() { CallListViewFunction(&ListView::sortCatalog); }
void MainWindow::updateCatalog() { CallListViewFunction(&ListView::updateCatalog); }
void MainWindow::moveCatalogFiles() { CallListViewFunction(&ListView::moveCatalogFiles); }
void MainWindow::deleteCatalogFiles() { CallListViewFunction(&ListView::deleteCatalogFiles); }

void MainWindow::trashFiles() {
  ImageView* image;
  SlideshowView* slideshow;
  ListView* list;
  if ((image = activeView<ImageView*>()) != nullptr)
    image->trashFile();
  else if ((slideshow = activeView<SlideshowView*>()) != nullptr)
    slideshow->UserAction(SlideshowView::SlideshowAction::TRASH_FILE);
  else if ((list = activeView<ListView*>()) != nullptr)
    list->trashCatalogFiles();
}

void MainWindow::slotListViewCurrentChanged(const std::string& catalogFilename, const std::string& filename) {
  ListView* list;
  QMdiSubWindow* subWindow;
  if ((list = activeView<ListView*>()) != nullptr
      && (subWindow = mdiArea->activeSubWindow()) != nullptr) {
    subWindow->setWindowTitle((catalogFilename + " (" + TivUtility::Basename(filename) + ")").c_str());
  }
}

void MainWindow::slotslideshowkeyPress(void* listviewSubwindowId,
				       SlideshowView::SlideshowAction result) {
  auto findsubwindow = [&](const void* subwindow) -> QMdiSubWindow* {
    QList<QMdiSubWindow*> subwindows = mdiArea->subWindowList();
    for (auto it=subwindows.cbegin(); it!=subwindows.cend(); ++it) {
      if (subwindow == *it)
	return *it;
    }
    return nullptr;
  };

  SlideshowView* slideshow;
  QMdiSubWindow* subwindow;
  ListView* listview;
  if ((subwindow = findsubwindow(listviewSubwindowId)) != nullptr
      && (listview = dynamic_cast<ListView*>(subwindow->widget())) != nullptr
      && (slideshow = activeView<SlideshowView*>()) != nullptr) {

    std::string path = listview->SlideshowkeyPress(slideshow, result);
    QMdiSubWindow *subWindow = mdiArea->activeSubWindow();
    if (subWindow && !path.empty())
      subWindow->setWindowTitle((TivUtility::Basename(path) + " (Slideshow)").c_str());
  }
}

void MainWindow::slideshowCatalog() {
  QMdiSubWindow* subWindow;
  ListView* listview;
  SlideshowView* slideshow;
  QMdiSubWindow* slideshowSubwindow;

  if ((subWindow = mdiArea->activeSubWindow()) != nullptr
      && (listview = dynamic_cast<ListView*>(subWindow->widget())) != nullptr
      && (slideshow = listview->slideshowCatalog()) != nullptr
      && (slideshowSubwindow = mdiArea->addSubWindow(slideshow)) != nullptr) {

    connect(slideshow, &SlideshowView::signalkeyPress, this, &MainWindow::slotslideshowkeyPress);
    // disconnect() unneeded because ~QObject() disconnects signals to & from the object

    slideshow->setId(subWindow);
    slideshowSubwindow->setAttribute(Qt::WA_DeleteOnClose);
    slideshowSubwindow->setWindowTitle((TivUtility::Basename(listview->filename(0)) + " (Slideshow)").c_str());
    slideshowSubwindow->show();
  }
}

void MainWindow::slideshowPrevious() {
  SlideshowView* slideshow = activeView<SlideshowView*>();
  if (slideshow)
    slideshow->UserAction(SlideshowView::SlideshowAction::PREVIOUS);
}
void MainWindow::slideshowNext() {
  SlideshowView* slideshow = activeView<SlideshowView*>();
  if (slideshow)
    slideshow->UserAction(SlideshowView::SlideshowAction::NEXT);
}
void MainWindow::slideshowFirst() {
  SlideshowView* slideshow = activeView<SlideshowView*>();
  if (slideshow)
    slideshow->UserAction(SlideshowView::SlideshowAction::FIRST);
}
void MainWindow::slideshowLast() {
  SlideshowView* slideshow = activeView<SlideshowView*>();
  if (slideshow)
    slideshow->UserAction(SlideshowView::SlideshowAction::LAST);
}
