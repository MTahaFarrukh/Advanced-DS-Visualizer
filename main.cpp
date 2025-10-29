#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QStackedWidget>
#include "homepage.h"
#include "menupage.h"
#include "theorypage.h"
#include "operationpage.h"
#include "treeinsertion.h"
#include "hashmapvisualization.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create main window with stacked widget
    QStackedWidget *mainWindow = new QStackedWidget();
    mainWindow->setWindowTitle("Advanced Data Structure Visualizer");
    mainWindow->setMinimumSize(900, 750);

    // Create pages
    HomePage *homePage = new HomePage();
    MenuPage *menuPage = new MenuPage();

    // Add pages to stacked widget
    int homePageIndex = mainWindow->addWidget(homePage);
    int menuPageIndex = mainWindow->addWidget(menuPage);

    // Start with home page
    mainWindow->setCurrentIndex(homePageIndex);

    // Variables to track pages
    TheoryPage *currentTheoryPage = nullptr;
    OperationPage *currentOperationPage = nullptr;
    TreeInsertion *currentTreeInsertion = nullptr;
    HashMapVisualization *currentHashMapVisualization = nullptr;
    int theoryPageIndex = -1;
    int operationPageIndex = -1;
    int visualizationPageIndex = -1;
    QString currentDataStructure = "";

    // Connect HomePage to MenuPage navigation
    QObject::connect(homePage, &HomePage::navigateToMenu, [mainWindow, menuPageIndex]() {
        mainWindow->setCurrentIndex(menuPageIndex);
    });

    // Connect MenuPage to TheoryPage navigation
    QObject::connect(menuPage, &MenuPage::dataStructureSelected,
                     [mainWindow, menuPageIndex, &currentTheoryPage, &theoryPageIndex,
                      &currentOperationPage, &operationPageIndex, &currentDataStructure,
                      &currentTreeInsertion, &currentHashMapVisualization, &visualizationPageIndex](const QString &dsName) {
                         // Store current data structure
                         currentDataStructure = dsName;

                         // Remove old theory page if exists
                         if (currentTheoryPage) {
                             mainWindow->removeWidget(currentTheoryPage);
                             currentTheoryPage->deleteLater();
                         }

                         // Create new theory page
                         currentTheoryPage = new TheoryPage(dsName);
                         theoryPageIndex = mainWindow->addWidget(currentTheoryPage);

                         // Connect back button to menu
                         QObject::connect(currentTheoryPage, &TheoryPage::backToMenu,
                                          [mainWindow, menuPageIndex]() {
                                              mainWindow->setCurrentIndex(menuPageIndex);
                                          });

                         // Connect "Try It Yourself" button to operation page
                         QObject::connect(currentTheoryPage, &TheoryPage::tryItYourself,
                                          [mainWindow, &currentOperationPage, &operationPageIndex,
                                           &theoryPageIndex, &currentDataStructure, &currentTreeInsertion,
                                           &currentHashMapVisualization, &visualizationPageIndex]() {
                                              // Remove old operation page if exists
                                              if (currentOperationPage) {
                                                  mainWindow->removeWidget(currentOperationPage);
                                                  currentOperationPage->deleteLater();
                                              }

                                              // Create new operation page
                                              currentOperationPage = new OperationPage(currentDataStructure);
                                              operationPageIndex = mainWindow->addWidget(currentOperationPage);

                                              // Connect back button to theory page
                                              QObject::connect(currentOperationPage, &OperationPage::backToMenu,
                                                               [mainWindow, theoryPageIndex]() {
                                                                   mainWindow->setCurrentIndex(theoryPageIndex);
                                                               });

                                              // Connect operation selection to visualization
                                              QObject::connect(currentOperationPage, &OperationPage::operationSelected,
                                                               [mainWindow, &currentTreeInsertion, &currentHashMapVisualization, &visualizationPageIndex,
                                                                &operationPageIndex, &currentDataStructure](const QString &operation) {
                                                                   // Remove old visualization pages if they exist
                                                                   if (currentTreeInsertion) {
                                                                       mainWindow->removeWidget(currentTreeInsertion);
                                                                       currentTreeInsertion->deleteLater();
                                                                       currentTreeInsertion = nullptr;
                                                                   }
                                                                   if (currentHashMapVisualization) {
                                                                       mainWindow->removeWidget(currentHashMapVisualization);
                                                                       currentHashMapVisualization->deleteLater();
                                                                       currentHashMapVisualization = nullptr;
                                                                   }

                                                                   // Create appropriate visualization based on data structure and operation
                                                                   if (currentDataStructure == "Binary Tree" && operation == "Insertion") {
                                                                       currentTreeInsertion = new TreeInsertion();
                                                                       visualizationPageIndex = mainWindow->addWidget(currentTreeInsertion);

                                                                       // Connect back button to return to operations
                                                                       QObject::connect(currentTreeInsertion, &TreeInsertion::backToOperations,
                                                                                        [mainWindow, operationPageIndex]() {
                                                                                            mainWindow->setCurrentIndex(operationPageIndex);
                                                                                        });

                                                                       mainWindow->setCurrentIndex(visualizationPageIndex);
                                                                   }
                                                                   else if (currentDataStructure == "Hash Table") {
                                                                       // For HashMap, any operation opens the same interactive visualization
                                                                       currentHashMapVisualization = new HashMapVisualization();
                                                                       visualizationPageIndex = mainWindow->addWidget(currentHashMapVisualization);

                                                                       // Connect back button to return to operations
                                                                       QObject::connect(currentHashMapVisualization, &HashMapVisualization::backToOperations,
                                                                                        [mainWindow, operationPageIndex]() {
                                                                                            mainWindow->setCurrentIndex(operationPageIndex);
                                                                                        });

                                                                       mainWindow->setCurrentIndex(visualizationPageIndex);
                                                                   }
                                                                   // TODO: Add other data structures and operations here
                                                               });

                                              // Show operation page
                                              mainWindow->setCurrentIndex(operationPageIndex);
                                          });

                         // Show theory page
                         mainWindow->setCurrentIndex(theoryPageIndex);
                     });

    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - mainWindow->width()) / 2;
    int y = (screenGeometry.height() - mainWindow->height()) / 2;
    mainWindow->move(x, y);
    mainWindow->show();

    return app.exec();
}
