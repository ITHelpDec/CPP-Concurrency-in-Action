#include <string>
#include <thread>

// incomplete - will not compile
void edit_document(const std::string &filename) {
    open_document_and_display_gui(filename);
    
    while (!done_editing()) {
        user_command cmd = get_user_input();
        
        if (cmd.type == open_new_document) {
            std::string const new_name = get_filename_from_user();
            std::thread t(edit_document, new_name);
            t.detach();
        } else {
            process_user_input(cmd);
        }
    }
}
