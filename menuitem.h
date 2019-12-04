
enum MenuActionEnabled { NEVER=1, ALWAYS=2, IMAGE=4, CATALOG=8, SLIDESHOW=16, ANY=32, ANY2ORMORE=64 };

#if 1
struct menuItem {
  const QString             itemName;
  const QObject* const      receiver = nullptr;
  const char*    const      method = nullptr;
  const QList<QKeySequence> keys = QList<QKeySequence>();
  const int                 enableMenu = NEVER;

  menuItem(const QString& name,
	   const QObject* const rcv,
	   const char*    const mthd,
	   const QList<QKeySequence>& k,
	   const int e) : itemName(name), receiver(rcv), method(mthd), keys(k), enableMenu(e)
  {  }

  menuItem(const QString& name,
	   const QObject* const rcv,
	   const char*    const mthd,
	   const QKeySequence k,
	   const int e) : itemName(name), receiver(rcv), method(mthd), keys({k}), enableMenu(e)
  {  }
  menuItem(const QString& name) : itemName(name) {  }

};

#else
struct menuItem {
  const QString&       itemName;
  const QObject* const receiver = nullptr;
  const char*    const method = nullptr;
  const QKeySequence&  keys = QKeySequence();
  const int            enableMenu = NEVER;
};
#endif
