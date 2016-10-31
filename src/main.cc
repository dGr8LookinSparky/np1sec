#include "jabberling.h"
#include "interface.h"
#include "room.h"

class JabberlingRoomInterface : public np1sec::RoomInterface
{
	public:
	JabberlingRoomInterface(Jabberite* settings): m_settings(settings) {}
	
	void send_message(const std::string& message);
	
	void disconnected();
	void user_joined(const np1sec::Identity& identity);
	void user_left(const np1sec::Identity& identity);
	
	protected:
	struct Jabberite* m_settings;
};

JabberlingRoomInterface* global_interface = 0;
std::string global_username;
np1sec::Room* global_room = 0;

std::string fingerprint(np1sec::PublicKey public_key)
{
	std::string output;
	for (size_t i = 0; i < sizeof(public_key.buffer); i++) {
		if (i > 0) {
			output += ':';
		}
		output += "0123456789abcdef"[(public_key.buffer[i] >> 4) & 0x0f];
		output += "0123456789abcdef"[(public_key.buffer[i] >> 0) & 0x0f];
	}
	return output;
}




void ui_connection_error(std::string message)
{
	jabberling_print("* Connection error: " + message + "\n");
}

void ui_connected(struct Jabberite* settings, std::string username, std::vector<std::string> users)
{
	UNUSED(users);
	
	global_interface = new JabberlingRoomInterface(settings);
	global_username = username;
	
	jabberling_print("* Connected as " + username + "\n");
}

void ui_joined(struct Jabberite* settings, std::string username)
{
	UNUSED(settings);
	
	jabberling_print("* " + username + " joined the room\n");
}

void ui_left(struct Jabberite* settings, std::string username)
{
	UNUSED(settings);
	
	jabberling_print("* " + username + " left the room\n");
	
	if (global_room) {
		global_room->user_left(username);
	}
}

void ui_message(struct Jabberite* settings, std::string username, std::string message)
{
	UNUSED(settings);
	
	if (global_room) {
		global_room->message_received(username, message);
	}
}



void JabberlingRoomInterface::send_message(const std::string& message)
{
	jabberling_send(m_settings, message);
}

void JabberlingRoomInterface::disconnected()
{
	jabberling_print("*** disconnected\n");
}

void JabberlingRoomInterface::user_joined(const np1sec::Identity& identity)
{
	jabberling_print("*** " + identity.username + "::" + fingerprint(identity.public_key) + " joined the room\n");
}

void JabberlingRoomInterface::user_left(const np1sec::Identity& identity)
{
	jabberling_print("*** " + identity.username + "::" + fingerprint(identity.public_key) + " left the room\n");
}



void ui_input(struct Jabberite* settings, std::string line)
{
	UNUSED(settings);
	
	if (line == "/connect") {
		if (global_room) {
			delete global_room;
		}
		
		np1sec::PrivateKey private_key = np1sec::PrivateKey::generate();
		jabberling_print("*** Joining the room as " + global_username + "::" + fingerprint(private_key.public_key()) + "\n");
		
		global_room = new np1sec::Room(global_interface, global_username, private_key);
		global_room->join();
	}
}

int main(int argc, char** argv)
{
	jabberling_main(argc, argv);
}
