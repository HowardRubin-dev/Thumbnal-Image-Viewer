#pragma once
#include <QObject>

class QImage;
class QPrinter;

class PrintImage : public QObject {
  Q_OBJECT
 public:
  PrintImage(const QImage& q);

  void Print();

 private:
  const QImage& qImage;

 private slots:
   void previewSlot(QPrinter*);
};
