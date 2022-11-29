//**********************************************************************
//* Formation of the interface of the main windown window
//*********************************************************************

#include "MainWindow.h"
// Link to port selector
extern QComboBox *pselector;

//************************************************
//* designer of the main window class
//************************************************
MainWindow::MainWindow(QString startfile) : QMainWindow()
{

  int i;

  this->resize(1000, 737);

  // Class for loading configs
  config = new QSettings("forth32", "qhuaweiflash", this);

  // Font for inscriptions on the main panel
  QFont font;
  font.setPointSize(14);
  font.setBold(true);
  font.setWeight(75);

  // FormTheIconOfTheMainWindow
  icon.addFile(QStringLiteral(":/icon.ico"), QSize(), QIcon::Normal, QIcon::Off);
  setWindowIcon(icon);

  // Window title
  settitle();

  // we get the dimensions of the main window from the config
  QRect mainrect = config->value("/config/MainWindowRect").toRect();
  if (mainrect != QRect(0, 0, 0, 0))
    setGeometry(mainrect);

  // We take out the width of the line of the HEX editor from the config

  // splitter - root widget of the window
  centralwidget = new QSplitter(Qt::Horizontal, this);
  setCentralWidget(centralwidget);

  // Left window - heading editor
  hdrpanel = new QWidget(centralwidget);
  centralwidget->addWidget(hdrpanel);
  centralwidget->setStretchFactor(0, 1);
  vlhdr = new QVBoxLayout(hdrpanel);

  // Firmware parameters
  hdlbl3 = new QLabel("File parameters");
  hdlbl3->setFont(font);
  vlhdr->addWidget(hdlbl3);

  QFormLayout *lfparm = new QFormLayout(0);
  vlhdr->addLayout(lfparm);
  // type of firmware file
  dload_id_selector = new QComboBox(centralwidget);
  lfparm->addRow("File type", dload_id_selector);
  // Flag of compression
  zflag_selector = new QCheckBox("Zlib-Cossack of Sections", centralwidget);
  vlhdr->addWidget(zflag_selector);
  // Fill the list of firmware codes
  for (i = 0; i < 8; i++)
  {
    dload_id_selector->insertItem(i, fw_description(i));
  }
  dload_id_selector->setCurrentIndex(0);

  // List of sections
  hdlbl1 = new QLabel("List of sections", hdrpanel);
  hdlbl1->setFont(font);
  vlhdr->addWidget(hdlbl1);

  partlist = new QListWidget(hdrpanel);
  vlhdr->addWidget(partlist);

  hdlbl2 = new QLabel("The heading of the section", hdrpanel);
  hdlbl2->setFont(font);
  vlhdr->addWidget(hdlbl2);

  // composer for editing parameters
  lphdr = new QFormLayout(0);
  vlhdr->addLayout(lphdr);
  // Elements of the editor header
  pcode = new QLineEdit(hdrpanel);
  pcode->setMaxLength(4);
  pcode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pcode->setReadOnly(true);
  lphdr->addRow("In the section of the division", pcode);

  Platform_input = new QLineEdit(hdrpanel);
  Platform_input->setMaxLength(8);
  Platform_input->setReadOnly(true);
  Platform_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lphdr->addRow("Platform", Platform_input);

  Date_input = new QLineEdit(hdrpanel);
  Date_input->setMaxLength(16);
  Date_input->setReadOnly(true);
  Date_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lphdr->addRow("Date", Date_input);

  Time_input = new QLineEdit(hdrpanel);
  Time_input->setMaxLength(16);
  Time_input->setReadOnly(true);
  Time_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lphdr->addRow("Time", Time_input);

  setdate = new QToolButton(hdrpanel);
  setdate->setText("Install the current date");
  vlhdr->addWidget(setdate);

  vlhdr->addWidget(new QLabel("Version of the firmware", hdrpanel));

  QSize qs = Time_input->sizeHint();
  qs.rwidth() *= 2;
  Version_input = new QLineEdit(hdrpanel);
  Version_input->setMaxLength(32);
  Version_input->setReadOnly(true);
  Version_input->resize(qs);
  Version_input->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  vlhdr->addWidget(Version_input);

  // Right window - sections editor
  edpanel = new QWidget(centralwidget);
  centralwidget->addWidget(edpanel);
  centralwidget->setStretchFactor(1, 5);
  EditorLayout = new QVBoxLayout(edpanel);

  // Raw-format buttons
  laymode = new QHBoxLayout(0);
  modebuttons = new QGroupBox("View mode");
  // modebuttons->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  modebuttons->setFlat(false);

  dump_mode = new QRadioButton("НЕХ-sweep");
  laymode->addWidget(dump_mode);

  structure_mode = new QRadioButton("Format");
  structure_mode->setChecked(true);
  laymode->addWidget(structure_mode);

  modebuttons->setLayout(laymode);
  modebuttons->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  EditorLayout->addWidget(modebuttons, 0);

  // Horizontal separator
  hframe = new QFrame(edpanel);
  hframe->setFrameStyle(QFrame::HLine);
  hframe->setLineWidth(2);
  EditorLayout->addWidget(hframe, 0);

  // Statusbar
  statusbar = new QStatusBar(this);
  setStatusBar(statusbar);

  // Port choice
  plbl = new QLabel("Modema port:");
  statusbar->addPermanentWidget(plbl);

  PortSelector = new QComboBox(centralwidget);
  statusbar->addPermanentWidget(PortSelector);

  RefreshPorts = new QToolButton(centralwidget);
  RefreshPorts->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
  statusbar->addPermanentWidget(RefreshPorts);

  // Restore the condition of the splitter
  centralwidget->restoreState(config->value("/config/splitter").toByteArray());

  // Main menu
  menubar = new QMenuBar(this);
  menu_file = new QMenu("File", menubar);
  menubar->addAction(menu_file->menuAction());

  menu_part = new QMenu("Sections", menubar);
  menu_part->setEnabled(true);
  menubar->addAction(menu_part->menuAction());

  menu_oper = new QMenu("Operations", menubar);
  menu_oper->setEnabled(true);
  menubar->addAction(menu_oper->menuAction());

  setMenuBar(menubar);

  // Prostarters of the main menu
  fileopen = new QAction("Open", this);
  fileopen->setShortcut(QKeySequence::Open);
  menu_file->addAction(fileopen);

  fileappend = new QAction("Add", this);
  fileappend->setEnabled(false);
  menu_file->addAction(fileappend);
  menu_file->addSeparator();

  filesave = new QAction("Save", this);
  filesave->setEnabled(false);
  filesave->setShortcut(QKeySequence::Save);
  menu_file->addAction(filesave);

  menu_file->addAction("Save as...", this, SLOT(save_as()));
  menu_file->addSeparator();

  // last open files
  QStringList recent = config->value("/recent/rfiles").toStringList();
  for (int i = 0; i < recent.count(); i++)
  {
    menu_file->addAction(recent.at(i), this, SLOT(open_recent_file()));
  }

  menu_file->addSeparator();

  file_exit = new QAction("Exit", this);
  file_exit->setShortcut(QKeySequence::Quit);
  menu_file->addAction(file_exit);
  //----------------
  part_store = new QAction("Remove with the heading", this);
  menu_part->addAction(part_store);

  part_extract = new QAction("Remove without a title", this);
  part_extract->setShortcut(QKeySequence("Ctrl+T"));
  menu_part->addAction(part_extract);

  part_replace = new QAction("Replace the image of the section", this);
  menu_part->addAction(part_replace);

  MoveUp = new QAction("Move up", this);
  MoveUp->setShortcut(QKeySequence("Ctrl+Up"));
  menu_part->addAction(MoveUp);

  MoveDown = new QAction("Move down", this);
  MoveDown->setShortcut(QKeySequence("Ctrl+Down"));
  menu_part->addAction(MoveDown);

  Delete = new QAction("Delete", this);
  Delete->setShortcut(QKeySequence("Ctrl+Del"));
  menu_part->addAction(Delete);

  part_copy_header = new QAction("Copy the title", this);
  menu_part->addAction(part_copy_header);
  //----------------
  Menu_Oper_flash = new QAction("Go through the modem", this);
  Menu_Oper_flash->setEnabled(false);
  Menu_Oper_flash->setShortcut(QKeySequence("Alt+B"));
  menu_oper->addAction(Menu_Oper_flash);

  Menu_Oper_USBDload = new QAction("Loading usbdloader", this);
  Menu_Oper_USBDload->setShortcut(QKeySequence("Alt+U"));
  menu_oper->addAction(Menu_Oper_USBDload);
  menu_oper->addSeparator();

  Menu_Oper_Reboot = new QAction("Reboot the modem", this);
  menu_oper->addAction(Menu_Oper_Reboot);
  menu_oper->addSeparator();

  Menu_Oper_signinfo = new QAction("Digital signature information", this);
  Menu_Oper_signinfo->setShortcut(QKeySequence("Alt+D"));
  Menu_Oper_signinfo->setEnabled(false);
  menu_oper->addAction(Menu_Oper_signinfo);

  // Installation of signal handlers
  connect(fileopen, SIGNAL(triggered()), this, SLOT(SelectFwFile()));
  connect(partlist, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(SelectPart()));
  connect(fileappend, SIGNAL(triggered()), this, SLOT(AppendFwFile()));
  connect(part_extract, SIGNAL(triggered()), this, SLOT(Menu_Part_Extract()));
  connect(part_store, SIGNAL(triggered()), this, SLOT(Menu_Part_Store()));
  connect(partlist, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(SelectPart()));
  connect(part_replace, SIGNAL(triggered()), this, SLOT(Menu_Part_Replace()));
  connect(file_exit, SIGNAL(triggered()), this, SLOT(close()));
  connect(filesave, SIGNAL(triggered()), this, SLOT(SaveFwFile()));
  connect(Delete, SIGNAL(triggered()), this, SLOT(Menu_Part_Delete()));
  connect(MoveUp, SIGNAL(triggered()), this, SLOT(Menu_Part_MoveUp()));
  connect(MoveDown, SIGNAL(triggered()), this, SLOT(Menu_Part_MoveDown()));
  connect(partlist, SIGNAL(currentRowChanged(int)), this, SLOT(Disable_EditHeader()));
  connect(Menu_Oper_flash, SIGNAL(triggered()), this, SLOT(Start_Flasher()));
  connect(Menu_Oper_Reboot, SIGNAL(triggered()), this, SLOT(Reboot_modem()));
  connect(Menu_Oper_USBDload, SIGNAL(triggered()), this, SLOT(usbdload()));
  connect(setdate, SIGNAL(clicked()), this, SLOT(set_date()));
  connect(Menu_Oper_signinfo, SIGNAL(triggered()), this, SLOT(ShowSignInfo()));
  connect(dump_mode, SIGNAL(toggled(bool)), this, SLOT(SelectPart()));
  connect(partlist, SIGNAL(currentRowChanged(int)), this, SLOT(SelectPart()));
  connect(part_copy_header, SIGNAL(triggered()), this, SLOT(HeadCopy()));
  connect(RefreshPorts, SIGNAL(clicked()), this, SLOT(find_ports()));

  // External link to the chosen of the port
  pselector = PortSelector;

  // Creating a class class class
  ptable = new ptable_list;

  // Filling the list of ports
  find_ports();

  // Knocking the pointers to the editors of the sections
  hexedit = 0;
  ptedit = 0;
  cpio = 0;

  // Open the file line file
  if (!startfile.isEmpty())
  {
    OpenFwFile(startfile);
  }
  partlist->setFocus();
}

//*****************************************
//* class destructor
//*****************************************
MainWindow::~MainWindow()
{

  QRect mainrect;

  ask_save();

  if (hexedit != 0)
    delete hexedit;
  if (kedit != 0)
    delete kedit;
  if (nvedit != 0)
    delete nvedit;
  delete ptable;

  // Geometry of the main windowndow
  mainrect = geometry();
  config->setValue("/config/MainWindowRect", mainrect);

  // Splitter geometry
  config->setValue("/config/splitter", centralwidget->saveState());
}
