#include "AReVi/arSystem.h"
#include "AReVi/scheduler.h"
using namespace AReVi;

#include "hLib/hLib.h"
using namespace hLib;

#include "editor.h"
// WARNING : sometimes, often under FC4, libgnome & libgnomeui folders are called
// libgnome-2.0 & libgnomeui-2.0. You have to make symlinks to theses for it to work !

#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-ui-init.h>

void
registerMotionBlendingEditorClasses(void);

namespace GtkTools {
void
registerGtkToolsClasses(void);
}

// ----------------------------------------------------------------

ArRef<Scheduler>
simulationInit(void) {
  registerMotionBlendingEditorClasses();
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
  GnomeProgram *my_app;
  my_app = gnome_program_init("Motion Blending Editor","0.0.1", LIBGNOMEUI_MODULE, argc, argv, NULL);
  
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
