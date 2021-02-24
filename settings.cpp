//#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include "settings.h"
#include "utility.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

  buttonC = new QRadioButton("Activate windows in C&reation order", this);
  buttonS = new QRadioButton("Activate windows in &Stacking order", this);
  buttonA = new QRadioButton("Activate windows in &Activation History order", this);
  const std::string activationOrder = TivUtility::loadSetting("WindowActivationOrder", "Creation");
  switch(activationOrder[0]) {
  case 'C':
  default:
    buttonC->setChecked(true);
    break;
  case 'S':
    buttonS->setChecked(true);
    break;
  case 'A':
    buttonA->setChecked(true);
    break;
  }

  QVBoxLayout* layout = new QVBoxLayout(this);

  layout->addWidget(buttonC);
  layout->addWidget(buttonS);
  layout->addWidget(buttonA);
  layout->addWidget(buttonBox);

  connect(buttonBox,  &QDialogButtonBox::accepted, this, &SettingsDialog::OK);
  connect(buttonBox,  &QDialogButtonBox::rejected, this, &QDialog::reject);
};

void SettingsDialog::OK() {
  TivUtility::saveSetting("WindowActivationOrder",
			  buttonS->isChecked() ? "Stacking" :
			  buttonA->isChecked() ? "Activation" : "Creation");
  QDialog::accept();
}
