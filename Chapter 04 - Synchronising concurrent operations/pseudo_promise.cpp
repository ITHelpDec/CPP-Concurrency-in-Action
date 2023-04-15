// keek...utter keek

#include <future>

void process_connections(connection_set &connections) {
    while (!done(connections)) {
        for (connection_iterator conn_it = connections.begin(); conn_it != connections.end(); ++conn_it) {
            
            if (connection->has_incoming_data()) {
                data_packet data = connection->incoming();
                std::promise<payload_type> &p = connection->get_promise(data.id);
                p.set_value(data.payload);
            }
            
            if (connection->has_outgoing_data()) {
                outgoing_packet data = connection->top_of_outgoing_queue();
                connection->send(data.payload);
                data.promise.set_value(true);
            }
            
        }
    }
}
