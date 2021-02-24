#include <QDialog>

class QRadioButton;

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  SettingsDialog(QWidget* parent);

private slots:
  void OK();
  //void Cancel();

private:
  QRadioButton* buttonC, * buttonS, * buttonA;
};
