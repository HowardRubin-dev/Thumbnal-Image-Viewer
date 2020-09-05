#include "sortdialog.h"

#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QKeyEvent>

SortDialog::SortDialog() {
  setGeometry(100, 100, 300, 200);
  setWindowTitle(tr("Sort"));

  sortFields = new QComboBox(this);
  sortFields->setGeometry(10, 20, 100, 50);
  sortFields->addItem(tr("Date"));
  sortFields->addItem(tr("Name"));
  sortFields->addItem(tr("Path"));
  sortFields->addItem(tr("Size"));

  radioAscending = new QRadioButton(tr("&Ascending"),this);
  radioAscending->setGeometry(120, 10, 100, 50);
  radioDescending = new QRadioButton(tr("Descending"),this);
  radioDescending->setGeometry(120,30, 100, 50);
  radioDescending->setChecked(true); // Default to descending date order

  ok = new QPushButton(tr("&Ok"),this);
  ok->setGeometry(20, 150, 30, 40);
  cancel = new QPushButton(tr("&Cancel"),this);
  cancel->setGeometry(90, 150, 70, 40);

  QObject::connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
  //QObject::connect(QKeySequence::Open, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

bool SortDialog::isAscending() {
  return radioAscending->isChecked();
}
std::string SortDialog::sortField() {
  return sortFields->currentText().toUtf8().constData();
}

#if 0
void SortDialog::keyReleaseEvent(QKeyEvent* e) {
  //printf("e->key() = 0x%x\n", e->key());
  if (e->key() == Qt::Key_Return
      || e->key() == Qt::Key_Enter) {
    return; // accept();
  } else {
    QDialog::keyReleaseEvent(e);
  }
}
#endif
