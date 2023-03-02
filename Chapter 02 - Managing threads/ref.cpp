#include <thread>

void update_data_for_widget(widget_id, widget_data&);

void oops_again(widget_id w) {
    widget_data data;
    
    // will not compile - arguments copied as rvalues
    // std::thread t(update_data_for_widget, w, data);
    std::thread t(update_data_for_widget, w, std::ref(data));
    
    display_status();
    t.join();
    process_widget_data(data);
}
