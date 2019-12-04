#include <printimage.h>
#include "utility.h"
#include <QPainter>
#include <QPrinter>
#include <QPrintPreviewDialog>

PrintImage::PrintImage(const QImage& q) : qImage(q) {
}

void PrintImage::Print() {
  QPrinter printer(QPrinter::HighResolution);
  printer.setPageSize(QPrinter::Letter);
  printer.setColorMode(QPrinter::Color);

  printer.setPageOrientation(QPageLayout::Landscape);
  double scaleL = TivUtility::getScale<const QRectF&>(qImage, printer.pageRect());

  printer.setPageOrientation(QPageLayout::Portrait);
  double scaleP = TivUtility::getScale<const QRectF&>(qImage, printer.pageRect());

  printer.setPageOrientation(scaleP >= scaleL ? QPageLayout::Portrait : QPageLayout::Landscape);

  QPrintPreviewDialog previewDialog(&printer, nullptr, Qt::Window); // TODO parent
  connect(&previewDialog, SIGNAL(paintRequested(QPrinter*)), SLOT(previewSlot(QPrinter*)));
  previewDialog.exec();
}

void PrintImage::previewSlot(QPrinter* printer) {
  double scale = TivUtility::getScale<const QRectF&>(qImage, printer->pageRect());

  QPainter painter(printer);
  painter.scale(scale, scale);
  painter.drawImage(0, 0, qImage);//,0,0,-1,-1,Qt::AutoColor);
}

#if 0
Print directly without preview:

  QPrintDialog printDialog(&printer);
  //printDialog.setOption(QAbstractPrintDialog::PrintShowPageSize);
  if (printDialog.exec()) {
    QPainter painter(&printer);

    printer.setPageOrientation(QPageLayout::Landscape);
    double xscale = printer.pageRect().width()/double(qImage.width()),
	   yscale = printer.pageRect().height()/double(qImage.height()),
	   scaleL = qMin(xscale, yscale);

    printer.setPageOrientation(QPageLayout::Portrait);
    xscale = printer.pageRect().width()/double(qImage.width());
    yscale = printer.pageRect().height()/double(qImage.height());
    double scaleP = qMin(xscale, yscale);

    if (scaleP >= scaleL) { // Portrait print is bigger than Landscape
      painter.scale(scaleP, scaleP);
      painter.drawImage(0, 0, *qImage);//,0,0,-1,-1,Qt::AutoColor);

    } else { // Landscape print is bigger than Portrait
      QTransform rotating;
      rotating.rotate(90);
      QImage rotatedImage = qImage.transformed(rotating);

      painter.scale(scaleL, scaleL);
      painter.drawImage(0, 0, rotatedImage);//,0,0,-1,-1,Qt::AutoColor);

      //painter.translate(qImage.width()/2, qImage.height()/2);
      //painter.rotate(20);
      //painter.translate(-qImage.width()/2, -qImage.height()/2);
    }
  }
#endif
