#include <QDialog>

class QComboBox;
class QRadioButton;
class QPushButton;

// For more on
// QXcbConnection: XCB error: 3 (BadWindow)
// See QT bug report: https://bugreports.qt.io/browse/QTBUG-56893

class SortDialog : public QDialog {
  Q_OBJECT

public:
  SortDialog();
  bool isAscending();
  std::string sortField();

  //void keyReleaseEvent(QKeyEvent* e) override;

private:
  QComboBox* sortFields /*= nullptr*/;
  QRadioButton* radioAscending /*= nullptr*/,
              * radioDescending /*= nullptr*/;
  QPushButton* ok /*= nullptr*/,
             * cancel /*= nullptr*/;
};
