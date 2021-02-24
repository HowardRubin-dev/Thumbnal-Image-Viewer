#include "catalog.h"
#include "catalogitem.h"
#include "listview.h"
#include "mainwindow.h"
#include "SlideshowView.h"
#include "trashfiles.h"
#include <QKeyEvent>
#include <QListView>
#include <QMessageBox>

ListView::ListView(MainWindow* mainwin, Catalog* cat)
  : QListView(), m_mainwin(mainwin), m_catalog(cat),
    mostRecentSortTime(QTime::currentTime().addSecs(-10)) {
  setViewMode(QListView::IconMode);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setModel(cat);
  setResizeMode(QListView::Adjust);
  setGridSize(QSize(130, 130));
  setIconSize(QSize(128,128));
  //setWrapping(true);
  //setSelectionRectVisible(true);
  //setUniformItemSizes(true);
  //setLayoutMode(QListView::Batched);
}

// First vector element is current item, other vector elements are selected items.
// Current item is usually also selected.
std::vector<QModelIndex> ListView::getSelections() {
  std::vector<QModelIndex> rv;

  QModelIndex ixCur = currentIndex();
  rv.push_back(ixCur);

  QModelIndexList list = selectedIndexes();
  std::for_each(list.cbegin(), list.cend(), [&](auto& ix) { rv.push_back(ix); });

  return rv;
}

std::string ListView::filename(int row /* = -1 */) const {
  if (row >= 0)
    return m_catalog ? m_catalog->filename(row) : "";
  
  QModelIndex ixCur;
  QStandardItem* item;
  CatalogItem* catitem;
  if (m_catalog
      && (ixCur = currentIndex()).isValid()
      && (item = m_catalog->itemFromIndex(ixCur)) != nullptr
      && (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {
    return catitem->ImagePath();
  } else {
    return "";
  }
}

void ListView::OpenCurrentImage() {
  QStandardItem* item;
  CatalogItem* catitem;
  QModelIndex ixCur;
  if (m_catalog && m_mainwin
      && (ixCur = currentIndex()).isValid()
      && (item = m_catalog->itemFromIndex(ixCur)) != nullptr
      && (catitem = dynamic_cast<CatalogItem*>(item)) != nullptr) {

    m_mainwin->openImageFile(catitem->ImagePath());
  }
}

QModelIndex ListView::moveCursor(QAbstractItemView::CursorAction cursorAction,
				 Qt::KeyboardModifiers modifiers) {
  if (cursorAction == MoveHome && modifiers == Qt::NoModifier) { // To beginning of line
    QModelIndex ixThis = currentIndex(), // Start with current
                ixLeft;
    int rowThis = ixThis.row(),
        rowLeft;
    int xThis = rectForIndex(ixThis).center().x(),
        xLeft;

    while ((rowLeft = rowThis-1) >= 0 // Move left as far as possible
	&& (ixLeft  = m_catalog->index(rowLeft, 0)).isValid()
	&& (xLeft   = rectForIndex(ixLeft).center().x()) < xThis) {
      rowThis = rowLeft;
      ixThis  = ixLeft;
      xThis   = xLeft;
    }
    return ixThis;

  } else if (cursorAction == MoveEnd && modifiers == Qt::NoModifier) { // To end of line
    const int nRows = m_catalog->rowCount();
    QModelIndex ixThis = currentIndex(), // Start with current
                ixRight;
    int rowThis = ixThis.row(),
        rowRight;
    int xThis = rectForIndex(ixThis).center().x(),
        xRight;

    while ((rowRight = rowThis+1) < nRows // Move right as far as possible
	&& (ixRight  = m_catalog->index(rowRight, 0)).isValid()
	&& (xRight   = rectForIndex(ixRight).center().x()) > xThis) {
      rowThis = rowRight;
      ixThis  = ixRight;
      xThis   = xRight;
    }
    return ixThis;
  }

#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
  // Standard arrow key movement stops at beginning or end of row in Qt 5.9
  // Make it instead wrap to previous or next row
  // Some later Qt versions wrap arrow key movement
  else if ((cursorAction == MoveLeft || cursorAction == MoveRight) // MovePrevious MoveNext
      && modifiers == Qt::NoModifier) {
    const QModelIndex ixCur = currentIndex();
    int newRow = ixCur.row();
    if (cursorAction == MoveLeft && newRow > 0)
      --newRow;
    else if (cursorAction == MoveRight && newRow < m_catalog->rowCount() - 1)
      ++newRow;

    QModelIndex newIndex = m_catalog->index(newRow, 0);

    return newIndex;
  }
#endif
  return QListView::moveCursor(cursorAction, modifiers);
}
void ListView::keyReleaseEvent(QKeyEvent* e) {
  QItemSelectionModel* model;

  switch (e->key()) {
  case Qt::Key_Return:
  case Qt::Key_Enter:
    // May have closed the sort dialog with the enter key so don't open an image in that case
    if (QTime::currentTime().addMSecs(-200) > mostRecentSortTime)
      OpenCurrentImage();
    break;
  case Qt::Key_Home:
    if (e->modifiers() == Qt::ControlModifier) {
      if ((model = selectionModel()) != nullptr)
	model->clear();
      QModelIndex ix = m_catalog->index(0, 0);
      setCurrentIndex(ix);
      return;
    }
    break;
  case Qt::Key_End:
    if (e->modifiers() == Qt::ControlModifier) {
      if ((model = selectionModel()) != nullptr)
	model->clear();
      QModelIndex ix = m_catalog->index(m_catalog->rowCount()-1, 0);
      setCurrentIndex(ix);
      return;
    }
    break;
  }

  QListView::keyReleaseEvent(e);
}
void ListView::mouseDoubleClickEvent(QMouseEvent* e) {
  // May have closed the sort dialog with double click so don't open an image in that case
  if (QTime::currentTime().addMSecs(-500) > mostRecentSortTime)
    OpenCurrentImage();
  QListView::mouseDoubleClickEvent(e);
}

void ListView::updateCatalog() {
  QModelIndex ixCur = currentIndex();
  int newSelection;

  if (m_catalog
      && (newSelection = m_catalog->update(ixCur)) >= 0) {

    QModelIndex ix = m_catalog->index(newSelection, 0);
    setCurrentIndex(ix);
  }
}

// Include new filename in main window title in MainWindow::slotListViewCurrentChanged()
void ListView::currentChanged(const QModelIndex &current, const QModelIndex &previous) {
  if (m_catalog)
    emit signalCurrentChanged(m_catalog->CatalogFileName(), filename(current.row()));

  QListView::currentChanged(current, previous);
}

void ListView::sortCatalog() {
  std::vector<QModelIndex> newSelections;
  if (m_catalog
      && !(newSelections = m_catalog->sort(getSelections())).empty()) {

    QModelIndex& ix = newSelections.front();
    if (ix.isValid())
      setCurrentIndex(ix);

    QItemSelectionModel* model = selectionModel();
    if (model) {
      for (auto it=++newSelections.cbegin(); it!=newSelections.cend(); ++it) {
	model->select(*it, QItemSelectionModel::Select);
      }
    }
    mostRecentSortTime = QTime::currentTime();
  }
}

void ListView::CallCatalogFunction(int (Catalog::*catalogFunction)(const std::vector<QModelIndex>& oldSelections)) {
  int newSelection;

  std::vector<QModelIndex> oldSelections = getSelections();
  if (m_catalog
      && oldSelections.size() > 1 // Current is first, selections after that
      && (newSelection = (m_catalog->*catalogFunction)(oldSelections)) >= 0) {
      
      QModelIndex ix = m_catalog->index(newSelection, 0);
      setCurrentIndex(ix);
  }
}
void ListView::moveCatalogFiles() { CallCatalogFunction(&Catalog::moveFiles); }
void ListView::trashCatalogFiles() { CallCatalogFunction(&Catalog::trashFiles); }
void ListView::deleteCatalogFiles() { CallCatalogFunction(&Catalog::deleteFiles); }

SlideshowView* ListView::slideshowCatalog() {
  if (m_catalog) {
    return new SlideshowView(m_catalog->filename(0));
  } else {
    return nullptr;
  }
}

std::string ListView::SlideshowkeyPress(SlideshowView* slideshow, SlideshowView::SlideshowAction result) {
  int row = slideshow->getRow();

  auto changeSlide = [&](int row) -> std::string {
    const std::string& path = m_catalog->filename(row);

    if (!path.empty())
      slideshow->newSlide(row, path);
    return path;
  };
  auto removeSlideFile = [&](int (Catalog::*catFunction)(const std::vector<QModelIndex>&)) {
    QModelIndex ix = m_catalog->index(row, 0);
    std::vector<QModelIndex> vix({ix, ix}); // Current & 1 selected for remove
    (m_catalog->*catFunction)(vix);
    int newSlideNum = std::min(row, m_catalog->rowCount()-1); // In case removing last slide
    return changeSlide(newSlideNum);
    // TODO new current slide
  };

  switch (result) {
  case SlideshowView::SlideshowAction::PREVIOUS:
    return changeSlide(std::max(row-1, 0));
  case SlideshowView::SlideshowAction::NEXT:
    return changeSlide(std::min(row+1, m_catalog->rowCount()-1));
  case SlideshowView::SlideshowAction::FIRST:
    return changeSlide(0);
  case SlideshowView::SlideshowAction::LAST:
    return changeSlide(m_catalog->rowCount() - 1);
  case SlideshowView::SlideshowAction::TRASH_FILE:
    return removeSlideFile(&Catalog::trashFiles);
  case SlideshowView::SlideshowAction::DELETE_FILE:
    return removeSlideFile(&Catalog::deleteFiles);
  default:
    break;
  }

  return "";
}
