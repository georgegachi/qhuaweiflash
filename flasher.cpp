#include <QtWidgets>

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <termios.h>
#include <unistd.h>

#include "sio.h"
#include "ptable.h"
#include "signver.h"

void flasher();

// Indicator to the class class of sections
extern ptable_list* ptable;
int32_t signsize;
// Index to an open serial port
extern int siofd; // fd для работы с Последовательным портом

// Size Bloka Dan, my transfer modem
// #define fblock 4096
#define fblock 2048

//****************************************************
//* Determination of the version of the firmware
//*
//* 0 - there is no answer to the command
//* 1 - version 2.0
//* -1 - The version is not 2.0
//****************************************************
int dloadversion() {

int res;  
int i;  
QString str;
uint8_t replybuf[1024];

res=atcmd("^DLOADVER?",replybuf);
if (res == 0) return 0; // нет ответа - уже HDLC
if (strncmp((char*)replybuf+2,"2.0",3) == 0) return 1;
for (i=2;i<res;i++) {
  if (replybuf[i] == 0x0d) replybuf[i]=0;
}  
str.sprintf("Incorrect version of the firmware monitor - %s",replybuf+2);
QMessageBox::critical(0,"Error",str);
return -1;
}


//***************************************************
// Sending the command of the beginning of the section
//
// Code - 32 -bit section code
// size - full size of the recorded section
//
//*  result:
// false - error
// True - the team adopted by the modem
//***************************************************
bool dload_start(uint32_t code,uint32_t size) {

uint32_t iolen;  
uint8_t replybuf[4096];
  
static struct __attribute__ ((__packed__)) {
  uint8_t cmd=0x41;
  uint32_t code;
  uint32_t size;
  uint8_t pool[3]={0,0,0};
} cmd_dload_init;

cmd_dload_init.code=htonl(code);
cmd_dload_init.size=htonl(size);
iolen=send_cmd((uint8_t*)&cmd_dload_init,sizeof(cmd_dload_init),replybuf);
if ((iolen == 0) || (replybuf[1] != 2)) return false;
else return true;
}  

//***************************************************
// Sending the section block
//
// blk - # block
// pimage - the address of the beginning of the image in memory
//
//*  result:
// false - error
// True - the team adopted by the modem
//***************************************************
bool dload_block(uint32_t part, uint32_t blk, uint8_t* pimage) {

uint32_t res,blksize,iolen;
uint8_t replybuf[4096];

static struct __attribute__ ((__packed__)) {
  uint8_t cmd=0x42;
  uint32_t blk;
  uint16_t bsize;
  uint8_t data[fblock];
} cmd_dload_block;  
  
blksize=fblock; // начальное значение размера блока
res=ptable->psize(part)-blk*fblock;  // размер оставшегося куска до конца файла
if (res<fblock) blksize=res;  // корректируем размер последнего блока

// номер блока
cmd_dload_block.blk=htonl(blk+1);
// размер блока
cmd_dload_block.bsize=htons(blksize);
// порция данных из образа раздела
memcpy(cmd_dload_block.data,pimage+blk*fblock,blksize);
// отсылаем блок в модем
iolen=send_cmd((uint8_t*)&cmd_dload_block,sizeof(cmd_dload_block)-fblock+blksize,replybuf); // отсылаем команду

if ((iolen == 0) || (replybuf[1] != 2)) {
  printf("\n sent block:\n");
  dump(&cmd_dload_block,sizeof(cmd_dload_block),0);
  printf("\n\n reply\n");
  dump(replybuf,iolen,0);
  fflush(stdout);
  return false;
}  
else return true;
}

  
//***************************************************
// completion of the section of the section
//
// code - section code
// size - section size
//
//*  result:
// false - error
// True - the team adopted by the modem
//***************************************************
bool dload_end(uint32_t code, uint32_t size) {

uint32_t iolen;
uint8_t replybuf[4096];

static struct __attribute__ ((__packed__)) {
  uint8_t cmd=0x43;
  uint32_t size;
  uint8_t garbage[3];
  uint32_t code;
  uint8_t garbage1[11];
} cmd_dload_end;

cmd_dload_end.code=htonl(code);
cmd_dload_end.size=htonl(size);
iolen=send_cmd((uint8_t*)&cmd_dload_end,sizeof(cmd_dload_end),replybuf);
if ((iolen == 0) || (replybuf[1] != 2)) {
//   printf("\n sent block:\n");
//   dump(&cmd_dload_end,sizeof(cmd_dload_end),0);
//   printf("\n\n reply\n");
//   dump(replybuf,iolen,0);
//   fflush(stdout);
  return false;
}  
else return true;
}  


//******************************************************************************* 
//* Запуск процесса прошивки
//******************************************************************************* 
void flasher() {

int32_t res,part;
uint32_t iolen,blk,maxblock;
uint8_t replybuf[4096];
QString txt;
unsigned char cmdver=0x0c;
uint8_t signflag=0,rebootflag=0;


// Modem response options for hdlc teams
unsigned char OKrsp[]={0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a};
  
QDialog* Flasher=new QDialog;
Flasher->setWindowTitle("Modem firmware");
QVBoxLayout* vl=new QVBoxLayout(Flasher);  

QFont font;
font.setPointSize(17);
font.setBold(true);
font.setWeight(75);

QLabel* lbl1=new QLabel("Modem firmware");
lbl1->setFont(font);
lbl1->setScaledContents(true);
lbl1->setStyleSheet("QLabel { color : blue; }");


vl->addWidget(lbl1,4,Qt::AlignHCenter);

QCheckBox* dsign = new QCheckBox("Use digital signature",Flasher);
vl->addWidget(dsign);

// Check the presence of the signature and deactivate the control buttons if it is not

if (signlen == -1) {
//   printf("\n no sign! \n");
  dsign->setChecked(0);
  dsign->setEnabled(0);
}
else {
  dsign->setChecked(1);
  dsign->setEnabled(1);  
}


QCheckBox* creboot = new QCheckBox("Reloading at the end of the firmware",Flasher);
creboot->setChecked(true);
vl->addWidget(creboot);

QDialogButtonBox* buttonBox = new QDialogButtonBox(Flasher);
buttonBox->setOrientation(Qt::Horizontal);
buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
buttonBox->addButton("Start",QDialogButtonBox::AcceptRole);
vl->addWidget(buttonBox,10,Qt::AlignHCenter);

QObject::connect(buttonBox, SIGNAL(accepted()), Flasher, SLOT(accept()));
QObject::connect(buttonBox, SIGNAL(rejected()), Flasher, SLOT(reject()));

// Run the dialogue
res=Flasher->exec();

// We take out the parameters of dialogue
signflag=dsign->isChecked();
rebootflag=creboot->isChecked();

// Удаляем текущий диалог
delete Flasher;
if (res != QDialog::Accepted) return;

Flasher=new QDialog; 

QFormLayout* glm=new QFormLayout(Flasher);

QLabel* pversion = new QLabel(Flasher);
glm->addRow("Protocol version:",pversion);

QLabel* cpart = new QLabel(Flasher);
glm->addRow("Current section:",cpart);

QProgressBar* partbar = new QProgressBar(Flasher);
partbar->setValue(0);
glm->addRow("Section:",partbar);

QProgressBar* totalbar = new QProgressBar(Flasher);
totalbar->setValue(0);
glm->addRow("Total:",totalbar);

Flasher->show();
  
// Настройка SIO
if (!open_port())  {
  QMessageBox::critical(0,"Error", "The serial port does not open");
  goto leave;
}  
  
tcflush(siofd,TCIOFLUSH);  // очистка выходного буфера

res=dloadversion();
if (res == -1) {
  QMessageBox::critical(0,"Error", "unsupported version of the firmware protocol");
  goto leave;
}

if (res == 0) {
  QMessageBox::critical(0,"Error", "Port is not in the firmware mode");
  goto leave;
}  

// цифровая подпись
if (signflag) { 
  res=send_signver();
  if (!res) {
    QMessageBox::critical(0,"Error", "error for checking digital signature");
    goto leave;
  }  
}  

// Входим в HDLC-режим
usleep(100000);
res=atcmd("^DATAMODE",replybuf);
if (res != 6) {
  QMessageBox::critical(0,"Error entry into HDLC", "The wrong answer to the ^Datamode command");
  goto leave;
}  
if (memcmp(replybuf,OKrsp,6) != 0) {
  QMessageBox::critical(0,"Error entry into HDLC", "Team ^Datamode is rejected by the modem");
  goto leave;
}  

iolen=send_cmd(&cmdver,1,(unsigned char*)replybuf);
if (iolen == 0) {
  QMessageBox::critical(0,"Error of the HDLC protocol", "It is impossible to get the version of the firmware protocol");
  goto leave;
}  
// отбрасываем начальный 7E если он есть в ответе
if (replybuf[0] == 0x7e) memcpy(replybuf,replybuf+1,iolen-1);

if (replybuf[0] != 0x0d) {
  QMessageBox::critical(0,"Error of the HDLC protocol", "The modem rejected the command of the version of the protocol");
  goto leave;
}  

// выводим версию протокола в форму
res=replybuf[1];
replybuf[res+2]=0;
txt.sprintf("%s",replybuf+2);
pversion->setText(txt);
QCoreApplication::processEvents();

// Главный цикл записи разделов
for(part=0;part<ptable->index();part++) {
 // прогресс-бар по разделам 
 totalbar->setValue(part*100/ptable->index());
 // выводим имя раздела в форму
 txt.sprintf("%s",ptable->name(part));
 cpart->setText(txt);

 // команда начала раздела
 if (!dload_start(ptable->code(part),ptable->psize(part))) {
  txt.sprintf("Section %s Rejected",ptable->name(part)); 
  QMessageBox::critical(0,"Error",txt);
  goto leave;
}  
    
maxblock=(ptable->psize(part)+(fblock-1))/fblock; // число блоков в разделе
// Поблочный цикл передачи образа раздела
for(blk=0;blk<maxblock;blk++) {
 // Прогрессбар блоков
 partbar->setValue(blk*100/maxblock);
 QCoreApplication::processEvents();

 // Отсылаем очередной блок
  if (!dload_block(part,blk,ptable->iptr(part))) {
   txt.sprintf("Block %i section %s Rejected",blk,ptable->name(part)); 
   QMessageBox::critical(0,"error",txt);
   goto leave;
 }  
}    

// закрываем раздел
 if (!dload_end(ptable->code(part),ptable->psize(part))) {
//   txt.sprintf("Ошибка закрытия раздела %s",ptable->name(part)); 
//   QMessageBox::critical(0,"ошибка",txt);
//   leave();
//   return 0;
 }  
} // конец цикла по разделам

// Выводим модем из HDLC или, если надо, перезагружаем модем.
if (rebootflag)   modem_reboot();
else end_hdlc();

totalbar->setValue(100);
partbar->setValue(100);
QCoreApplication::processEvents();
QMessageBox::information(0,"OK","The recording is completed without errors");

// Завершаем процесс
leave:

close_port();

delete Flasher;
 
}
