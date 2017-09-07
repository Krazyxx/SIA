#include "viewer.h"

#include <filesystem/resolver.h>

int main(int argc, char *argv[])
{
    getFileResolver()->prepend(DATA_DIR);

    try {
        nanogui::init();
        Viewer *screen = new Viewer();;

        if (argc == 2) {
            /* load file from the command line */
            filesystem::path path(argv[1]);

            if(path.extension() == "scn") { // load scene file
                screen->loadScene(argv[1]);
            }else if(path.extension() == "exr") { // load OpenEXR image
                screen->loadImage(argv[1]);
            }
        }

        /* Enter the application main loop */
        nanogui::mainloop();

        delete screen;
        nanogui::shutdown();
    } catch (const std::exception &e) {
        cerr << "Fatal error: " << e.what() << endl;
        return -1;
    }
    return 0;
}
