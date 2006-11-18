#include "AReVi/arSystem.h"
#include "AReVi/scheduler.h"
using namespace AReVi;

#include "hLib/hLib.h"
using namespace hLib;

#include "editor.h"

void
registerConstraintEditorClasses(void);

namespace GtkTools {
void
registerGtkToolsClasses(void);
}

// ----------------------------------------------------------------

ArRef<Scheduler>
simulationInit(void) {
  registerConstraintEditorClasses();
  GtkTools::registerGtkToolsClasses();

  //  ArRef<Scheduler> scd = new_RealTimeScheduler(1e-3);
  ArRef<Scheduler> scd = RealTimeScheduler::NEW(1e-3);

  //  ArRef<Editor> e = new_Editor();
  ArRef<Editor> e = Editor::NEW();
  e->setTransient(false);

  return(scd);
}

int
main(int argc, char ** argv) {
  if(!gtk_init_check(&argc, &argv)) {
    cerr << "Cannot initialize GTK" << endl;
    return(-1);
  }

  ArSystem arevi(argc,argv);
  ArSystem::loadPlugin("Imlib2ImageLoader");
  ArSystem::loadPlugin("MagickImageLoader");
  hLibInit();

  if(ArSystem::enterSimulation(&simulationInit)) {
    while(ArSystem::simulationStep()) {
      while(gtk_events_pending()) {
	gtk_main_iteration();
      }
    }
  }

  hLibDeInit();
  return 0;
}
