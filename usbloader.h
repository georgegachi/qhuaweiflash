#include <QtWidgets>

//****************************************************************
//* class of the dialog box
//****************************************************************
class usbldialog: public QDialog {

Q_OBJECT
public:
  QLineEdit* fname=0;
  QLineEdit* ptfname=0;
  
  // Designer
  usbldialog(): QDialog(0){};

  // Destructor
  ~usbldialog() {
    if (fname != 0) delete fname;
    if (ptfname != 0) delete ptfname;
  }
  
public slots: 
  void browse();
  void ptbrowse();
  void ptclear();
};  

//****************************************************************
// Tailor title
//****************************************************************
struct lhead{
  uint32_t lmode;  // Launch mode: 1 - direct start, 2 - through restart A -Core
  uint32_t size;   // component size
  uint32_t adr;    // The address of the component loading in memory
  uint32_t offset; // displacement to the component from the beginning of the file
};

void usbload();