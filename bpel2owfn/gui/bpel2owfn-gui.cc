#include <QApplication>

#include "ui_dialog.h"
#include "gui_organizer.h"


int main(int argc, char *argv[])
{  
  // the GUI application
  QApplication app(argc, argv);
  
  // the main window widget
  QMainWindow widget;
  
  // the derived (customized) main window
  Ui_MainWindow test;
  
  // set up the customized main window
  test.setupUi(&widget);
  
  // the organizer object
  GUI_Organizer organizer;
  
  
  // the slider for the structural reduction
  QObject::connect(test.reduction_horizontalSlider, SIGNAL(valueChanged(int)), &organizer, SLOT(set_reductionLevel(int)));
  QObject::connect(&organizer, SIGNAL(reductionLevel_changed(QString)), test.reduction_label, SLOT(setText(QString)));

  // the command line
  QObject::connect(test.commandLine_checkBox, SIGNAL(toggled(bool)), &organizer, SLOT(set_longOptions(bool)));
  QObject::connect(&organizer, SIGNAL(commandLine_changed(QString)), test.commandLine_lineEdit, SLOT(setText(QString)));
  
  // the output file
  QObject::connect(test.fileFormatFiona_checkBox, SIGNAL(toggled(bool)), &organizer, SLOT(set_fileFormatFiona(bool)));  
  QObject::connect(test.fileFormatLola_checkBox, SIGNAL(toggled(bool)), &organizer, SLOT(set_fileFormatLola(bool)));  
  QObject::connect(test.fileFormatPnml_checkBox, SIGNAL(toggled(bool)), &organizer, SLOT(set_fileFormatPnml(bool)));  
  QObject::connect(test.fileFormatDot_checkBox, SIGNAL(toggled(bool)), &organizer, SLOT(set_fileFormatDot(bool)));  
  QObject::connect(test.fileFormat_lineEdit, SIGNAL(textChanged(QString)), &organizer, SLOT(set_outputFileName(QString)));
  
  // the patterns
  QObject::connect(test.patternSmall_radioButton, SIGNAL(toggled(bool)), &organizer, SLOT(set_patternSmall(bool)));
  QObject::connect(test.patternNofhfaults_radioButton, SIGNAL(toggled(bool)), &organizer, SLOT(set_patternNofhfaults(bool)));
  QObject::connect(test.patternStandardfaults_radioButton, SIGNAL(toggled(bool)), &organizer, SLOT(set_patternStandardfaults(bool)));  
  QObject::connect(test.patternVariables_checkBox, SIGNAL(toggled(bool)), &organizer, SLOT(set_patternVariables(bool)));  
  QObject::connect(&organizer, SIGNAL(no_variables_possible(bool)), test.patternVariables_checkBox, SLOT(setEnabled(bool)));
  
  
  // initialize the command line
  organizer.set_commandLine();
  
  // show the main window widget
  widget.show();
  
  return app.exec();
}
