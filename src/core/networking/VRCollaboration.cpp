#include "VRCollaboration.h"
#include "core/utils/toString.h"

#include "tcp/VRICEclient.h"
#include "tcp/VRTCPClient.h"
#include "core/objects/sync/VRSyncNode.h"
#include "core/objects/geometry/sprite/VRSprite.h"
#include "core/objects/material/VRMaterial.h"
#include "core/objects/VRCamera.h"
#include "core/tools/VRAnnotationEngine.h"
#include "core/scene/sound/VRMicrophone.h"
#include "core/scene/sound/VRSound.h"
#include "core/scene/VRSceneManager.h"
#include "core/scene/VRScene.h"
#include "core/setup/VRSetup.h"
#include "core/setup/devices/VRServer.h"
#include "core/utils/system/VRSystem.h"

#ifndef WITHOUT_IMGUI
#include "core/gui/VRGuiConsole.h"
#endif

#define WEBSITEV(...) #__VA_ARGS__
#define WEBSITE(...) WEBSITEV(__VA_ARGS__)

using namespace OSG;

VRCollaboration::VRCollaboration(string name) : VRObject(name) {}
VRCollaboration::~VRCollaboration() {}

VRCollaborationPtr VRCollaboration::create(string name) {
    auto c = VRCollaborationPtr( new VRCollaboration(name) );
    c->init();
    return c;
}

VRCollaborationPtr VRCollaboration::ptr() { return static_pointer_cast<VRCollaboration>(shared_from_this()); }

void VRCollaboration::init() {
    ice = VRICEClient::create();
	ice->onEvent(bind(&VRCollaboration::onIceEvent, this, placeholders::_1));

    syncNode = VRSyncNode::create("syncNode");
	VRObject::addChild(syncNode);
	syncNode->onEvent(bind(&VRCollaboration::onSyncNodeEvent, this, placeholders::_1));

    mike = VRMicrophone::create();
	mike->pauseStreaming(true);

    initUI();
}

void VRCollaboration::onSyncNodeEvent(string e) {
    cout << "VRCollaboration::onSyncNodeEvent, event: " << e << endl;
    if (startsWith(e, "dropUser|")) {
        string uID = splitString(e, '|')[1];
        cout << " drop user with uID: " << uID << endl;
        cout << "  ICE users: " << toString(ice->getUsers()) << endl;
        ice->removeLocalUser(uID);
        updateUsersWidget();
    }
}

void VRCollaboration::addChild(VRObjectPtr child, bool osg, int place) {
    syncNode->addChild(child, osg, place);
}

void VRCollaboration::subChild(VRObjectPtr child, bool osg) {
    syncNode->subChild(child, osg);
}

void VRCollaboration::setServer(string uri) {
	ice->setTurnServer(uri);
}

void VRCollaboration::setupLocalServer() {
    string D = VRSceneManager::get()->getOriginalWorkdir();
    string folder = D+"/ressources/PolyServ";
    if (!exists(folder+"/.git"))
        systemCall("git clone https://github.com/Victor-Haefner/PolyServ.git \"" + folder + "\"");
    setServer("http://localhost:5500/ressources/PolyServ");
}

void VRCollaboration::debugAvatars() {
    cout << endl << " --- debugAvatars --- " << endl;
    auto remotes = syncNode->getRemotes();
    cout << " " << remotes.size() << " remotes" << endl;
    for (auto rID : remotes) {
        auto remote = syncNode->getRemote(rID);
        auto& avatar = remote->getAvatar();
        cout << "  remote " << rID << ", avatar: " << avatar.name << endl;
        cout << "  avatar local dev IDs: " << avatar.localHeadID << ", " << avatar.localDevID << ", " << avatar.localAnchorID << endl;
        cout << "  avatar local trans IDs: " << avatar.tHeadID << ", " << avatar.tDevID << ", " << avatar.tAnchorID << endl;
        cout << "  avatar remote IDs: " << avatar.remoteHeadID << ", " << avatar.remoteDevID << ", " << avatar.remoteAnchorID << endl;
    }
    cout << endl;
}

void VRCollaboration::setAvatarDevices(VRTransformPtr head, VRTransformPtr hand, VRTransformPtr handGrab) {
    if (!handGrab) handGrab = hand;
	syncNode->setAvatarBeacons(head, hand, handGrab);
}

void VRCollaboration::setAvatarGeometry(VRTransformPtr torso, VRTransformPtr leftHand, VRTransformPtr rightHand, float scale) {
    avatarTorso = torso;
    avatarHandLeft = leftHand;
    avatarHandRight = rightHand;
    avatarScale = scale;
}

void VRCollaboration::setupUserAvatar(string rID, string name) {
	auto avatar = VRTransform::create("avatar");
	VRObject::addChild(avatar);

	auto avatarScaleNode = VRTransform::create("avatarScale");
	avatarScaleNode->setScale( avatarScale );
	avatar->addChild(avatarScaleNode);

	auto rightHandContainer = VRTransform::create("rightHandContainer");
	auto anchor = VRTransform::create("anchor");
	rightHandContainer->addChild(anchor);
	avatarScaleNode->addChild(rightHandContainer);

	if (avatarTorso) {
        auto torso = dynamic_pointer_cast<VRTransform>(avatarTorso->duplicate());
        torso->setFrom(Vec3d(0,5,-0.8));
        avatarScaleNode->addChild(torso);
	}

    if (avatarHandLeft) {
        auto leftHand = dynamic_pointer_cast<VRTransform>(avatarHandLeft->duplicate());
        leftHand->rotate(1.5707/90*40, Vec3d(1,0,0)) ; // rotate 40° around z-Axis
        leftHand->rotate(1.5707*2, Vec3d(0,1,0));
        leftHand->translate(Vec3d(-2.4,0,0));
        avatarScaleNode->addChild(leftHand);
    }

    if (avatarHandRight) {
        auto rightHand = dynamic_pointer_cast<VRTransform>(avatarHandRight->duplicate());
        rightHand->rotate(1.5707*2, Vec3d(0,0,1));
        rightHand->translate(Vec3d(2.4,0,0));
        rightHandContainer->addChild(rightHand);
    }

	auto label = VRAnnotationEngine::create("nameTag");
	label->setSize(avatarScale);
	label->setBillboard(true);
	label->set(0, Vec3d(0,9,0)*avatarScale, name);
	avatar->addChild(label);

	auto job = bind(&VRSyncNode::addRemoteAvatar, syncNode.get(), rID, name, avatar, rightHandContainer, anchor);
	VRUpdateCbPtr cb = VRUpdateCb::create("syncNode-addRemoteAvatar", job);
	VRScene::getCurrent()->queueJob(cb);
}

void VRCollaboration::connectTCP(string origin, bool isWindows) {
#ifndef WITHOUT_IMGUI
    VRConsoleWidget::get("Collaboration")->write( "Collab: connect TCP sync node and audio, setup avatar of origin "+origin+"'\n");
#endif

    auto cli = ice->getClient(origin, VRICEClient::SCENEGRAPH);
    auto client = dynamic_pointer_cast<VRTCPClient>(cli);

    if (!client) {
#ifndef WITHOUT_IMGUI
        VRConsoleWidget::get("Collaboration")->write( "Collab: no TCP client found for origin '"+origin+"'\n", "red");
#endif
        return;
    }

    if (!client->connected()) {
#ifndef WITHOUT_IMGUI
        VRConsoleWidget::get("Collaboration")->write( "Collab: TCP client found for origin '"+origin+"' is not connected!\n", "red");
#endif
        return;
    }

	auto rID = syncNode->addTCPClient(client);
	auto name = ice->getUserName(origin);
	setupUserAvatar(rID, name);

	auto audioClient = ice->getClient(origin, VRICEClient::AUDIO);
	mike->startStreamingOver(audioClient);
	//mike->pauseStreaming(false);
	voices[origin] = VRSound::create();
	voices[origin]->playPeerStream(audioClient, isWindows);
}

void VRCollaboration::onIceEvent(string m) {
	if (m == "users changed") updateUsersWidget();

	if ( startsWith(m, "message|") ) {
        auto data = splitString(m, '|');
		if (data.size() < 2) return;
		auto message = data[1];
        data = splitString(message, '\n');
		if (data.size() < 4) return;
		string origin = data[2];
		string content = data[3];

		if (startsWith(content, "CONREQ") ) {
            string name = ice->getUserName(origin);
			sendUI("conReq", "configMessage|"+name+"|"+origin);
			connectionInWidget->show();
			connReqNet[origin] = parseSubNet(content);
			auto data2 = splitString(content, 'I');
			connReqSystem[origin] = data2[1];
		}

		if (startsWith(content, "CONACC") ) {
			auto data2 = splitString(content, 'I');
			bool isWindows = bool(data2[1] == "win");
			finishConnection(origin, isWindows, parseSubNet(content));
		}
    }
}

void VRCollaboration::sendUI(string widget, string data) {
	if (!gui_sites.count(widget)) return;
	auto dev = VRSetup::getCurrent()->getDevice("server1");
	auto server = dynamic_pointer_cast<VRServer>(dev);
	if (server) server->answer(gui_sites[widget], data);
}

void VRCollaboration::updateUsersWidget() {
	sendUI("usersList", "clearUsers");
	for (auto& usr : ice->getUsers()) {
        string uid = usr.first;
        string name = usr.second;
		sendUI("usersList", "addUser|"+name+"|"+uid);
	};

	auto uid = ice->getID();
	if (uid != "" ) sendUI("usersList", "setUserStats|"+uid+"|#23c");
}

void VRCollaboration::initUI() {
	gui_sites.clear();

	auto dev = VRSetup::getCurrent()->getDevice("server1");
	auto server = dynamic_pointer_cast<VRServer>(dev);
    auto port = server->getPort();
    auto cam = VRScene::getCurrent()->getActiveCamera();

    ui_dev_cb = VRDeviceCb::create( "CollabUI", bind(&VRCollaboration::handleUI, this, _1 ) );
    server->newSignal(-1, 0)->add( ui_dev_cb );

    auto serveSite = [&](string name, string content) {
        server->addWebSite(name, content);
    };

	auto addWidget = [&](string name, string site, int res, float W, float H, Vec3d pos) {
		auto w = VRSprite::create(name);
		w->setPersistency(0);
		cam->addChild(w);
		w->setFrom(pos);
		w->setSize(W,H);
		w->webOpen("localhost:"+toString(port)+"/"+site, res, float(W)/H);
		w->getMaterial()->enableTransparency();
		return w;
	};

	serveSite("uiCSS", uiCSS);
	serveSite("userlistSite", userlistSite);
	serveSite("userNameSite", userNameSite);
	serveSite("connectionInSite", connectionInSite);

	userlist = addWidget("userlist", "userlistSite", 200, 3, 6, Vec3d(-8,0,-10));
	userNameWidget = addWidget("userNameWidget", "userNameSite", 200, 3, 1.5, Vec3d(0,0,-10));
	connectionInWidget = addWidget("connectionInWidget", "connectionInSite", 200, 3, 1.5, Vec3d(0,1.5,-10));
	userNameWidget->hide();
	userlist->hide();
	connectionInWidget->hide();
}

bool VRCollaboration::handleUI(VRDeviceWeakPtr wdev) {
    auto dev = wdev.lock();
	auto m = dev->getMessage();
	auto n = splitString(m, ';');

	if (startsWith(m, "register")) {
        auto data = splitString(m, ' ');
        if (data.size() == 2) {
            string widget = data[1];
            gui_sites[widget] = dev->key();
            if (widget == "usersList") updateUsersWidget();
            if (widget == "namebox") userNameWidget->show();
        }
	}

	if (startsWith(m, "setName") ) {
        auto data = splitString(m, '|');
        if (data.size() >= 2) {
            userName = data[1];
            userNameWidget->hide();
            userlist->show();
            ice->setName(userName, false);
            updateUsersWidget();
        }
	}

	if (startsWith(m, "onUserClick") ) {
        auto data = splitString(m, '|');
        if (data.size() >= 2) {
            sendUI("usersList", "setUserStats|"+data[1]+"|#fa0");
            string net = getSubnet();
#ifdef _WIN32
			ice->send(data[1], "CONREQIwinI"+net);
#else
			ice->send(data[1], "CONREQIlnxI"+net);
#endif
        }
	}

	if (startsWith(m, "setMute") ) {
		auto data = splitString(m, '|');
        if (data.size() >= 2) {
            bool b = bool(data[1] == "true");
            cout << "setMute: " << b << endl;
            mike->pauseStreaming(b);
        }
	}

	if (startsWith(m, "connectionAccept") ) {
        auto data = splitString(m, '|');
        auto origin = data[1];
		bool isWindows = bool(connReqSystem[origin] == "win");
        acceptConnection(origin, isWindows);
        connectionInWidget->hide();
	}

	if (m == "connectionRefuse" ) connectionInWidget->hide();

	return true;
}

string VRCollaboration::getSubnet() {
    string net = ice->getID();
    for (auto c : ice->getClients()) net += "I"+c.first;
    return net;
}

vector<string> VRCollaboration::parseSubNet(string net) {
    auto data = splitString(net, 'I');
    data.erase(data.begin()); // removes "CONREQ" or "CONACC"
    data.erase(data.begin()); // removes "win" or "lnx"
    return data;
}

void VRCollaboration::acceptConnection(string origin, bool isWindows) {
#ifndef WITHOUT_IMGUI
    VRConsoleWidget::get("Collaboration")->write( "Collab: accepting incomming connection, connect ice to origin!\n");
#endif
    string net = getSubnet();
    for (auto node : connReqNet[origin]) {
        sendUI("usersList", "setUserStats|"+node+"|#2c4");
        ice->connectTo(node, false);
#ifdef _WIN32
        ice->send(node, "CONACCIwinI"+net);
#else
        ice->send(node, "CONACCIlnxI"+net);
#endif
        connectTCP(node, isWindows);
    }
}

void VRCollaboration::finishConnection(string origin, bool isWindows, vector<string> net) {
#ifndef WITHOUT_IMGUI
    VRConsoleWidget::get("Collaboration")->write( "Collab: got CONACC, connect ice to origin!\n");
#endif
    for (auto node : net) {
        sendUI("usersList", "setUserStats|"+node+"|#2c4");
        ice->connectTo(node, false);
        connectTCP(node, isWindows);
        if (node != origin) {
#ifdef _WIN32
			ice->send(node, "CONACCIwinI"+ice->getID());
#else
			ice->send(node, "CONACCIlnxI"+ice->getID());
#endif
		}
    }
}

string VRCollaboration::uiCSS = WEBSITE(
body {
	font-size: 8vw;
	color: white;
	margin: 0;
	background-color: #23272a;
	width: 100vw;
	height: 100vh;
}

.emptyBG {
	margin:5px;
	color:rgba(0,255,0,0.5);
	background:rgba(255,255,255,0.0);
	overflow: hidden;
}

input {
	border-radius:5vw;
	border: 1vw solid rgba(0,150,0,1);
	margin-bottom: 5vw;
	padding: 3vw;
}

.remoteUsername {
	color: yellow;
	font-weight: bold;
}

.selfUsername {
	color: #ffffff;
	font-weight: bold;
} #userlistContainer {
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	text-align: center;
	width: 100vw;
	height: calc(100vh - 20vw);
 	background-color: #2c2f33;
} #toolbar {
	display: flex;
	flex-direction: row;
	align-items: center;
	width: 100vw;
	height: 20vw;
 	background-color: #2c2f66;
}

button {
	background-color: #888;
	color: white;
	border: none;
	text-align: center;
	font-size: 8vw;
	height: 20vw;
}

button:hover, .open-button:hover {
	opacity: 1;
}
);

string VRCollaboration::userNameSite = WEBSITE(
<!DOCTYPE html>
<html>

<head>
	<link rel="stylesheet" href="uiCSS">
</head>

<body class='emptyBG'>
	<script>
		var ws = new WebSocket('ws://localhost:$PORT_server1$');
		ws.onopen = function() { send('register namebox'); };
		function send(m) { ws.send(m); };
	</script>

	<input id='field' class='background' value='Enter Name'></input>

	<script>
		input = document.getElementById("field");
		input.addEventListener("keyup", function(event) {
			if (event.keyCode === 13) send('setName|'+input.value);
		});
		input.addEventListener("click", function(event) {
			input.value = "";
		});
	</script>
</body>
</html>
);

string VRCollaboration::connectionInSite = WEBSITE(
<!DOCTYPE html>
<html>

<head>
    <link rel="stylesheet" href="uiCSS">\n
	<style>\n
		button { width: 50%; }\n
	</style>\n
</head>

<body>
	<script>\n
        var reqOrigin = 'unknown';\n
		var ws = new WebSocket('ws://localhost:$PORT_server1$');\n
		ws.onopen = function() { send('register conReq'); };\n
 		ws.onmessage = function(m) { if(m.data) handle(m.data); };\n
		function send(m) { ws.send(m); };\n

 		function handle(m) {\n
 			if (m.startsWith('configMessage|')) {\n
 				var data = m.split('|');\n
                reqOrigin = data[2];
                var msg = 'Accept incomming connection from '+data[1]+' ('+reqOrigin+')?';\n
                document.getElementById('userlistContainer').innerHTML = msg;\n
 			}\n
 		};\n\n
	</script>\n

	<div id='userlistContainer'>Accept incomming connection?</div>
	<div id='toolbar'>
		<button onclick='send("connectionAccept|"+reqOrigin)'>Yes</button>
		<button onclick='send("connectionRefuse")'>No</button>
	</div>
</body>
</html>
);

string VRCollaboration::userlistSite = WEBSITE(
<html>\n
	<head>\n
		<link rel="stylesheet" href="gui/font-awesome-4.5.0/css/font-awesome.min.css">\n
		<link rel="stylesheet" href="uiCSS">\n
	</head>\n
	<body>\n
		<div id="userlistContainer"></div>\n\n
		<div id="toolbar">\n
            <button onclick="toggleMute();" style="width:33%;"><i id="mikeIcon" class="fa fa-microphone-slash"></i></button>\n
		</div>\n\n

		<script>\n
		var websocket = new WebSocket('ws://localhost:$PORT_server1$');\n
		websocket.onopen = function() { send('register usersList'); };\n
 		websocket.onerror = function(e) {};\n
 		websocket.onmessage = function(m) { if(m.data) handle(m.data); };\n
 		websocket.onclose = function(e) {};\n\n

 		function send(m) { websocket.send(m); };\n\n

 		function handle(m) {\n
 			if (m == 'clearUsers') document.getElementById('userlistContainer').innerHTML = "";\n\n

 			if (m.startsWith('addUser|')) addUser(m.split('|')[1], m.split('|')[2]);\n\n

 			if (m.startsWith('setUserStats|')) {\n
 				var data = m.split('|');\n
 				setUserStats(data[1], data[2]);\n
 			}\n
 		};\n\n

 		function setUserStats(name, params) {\n
 			btn = document.getElementById(name);\n
 			if (btn) {\n
                params = params.split(' ');\n
                var color = params[0];\n
                console.log('setUserStats '+name+" "+color);\n
                btn.style.background = color;\n
 			}\n
 		}\n\n

 		function addUser(name, uid) {\n
			let btn = document.createElement("button");\n
			btn.setAttribute("id", uid);\n
			btn.className = "button";\n
			btn.innerHTML = name;\n
			btn.style.width = '80%';\n
			document.getElementById("userlistContainer").appendChild(btn);\n
			btn.onclick = function () {\n
				console.log("Button " + btn.id + " is clicked");\n
				send('onUserClick|'+btn.id);\n
			};\n
 		}\n\n

		var username = "Anonymous";\n
		var seperator = "<chatModule_seperator>";\n
		var messageIndex = 0;\n\n

		function sendMessage(message) {\n
			send('chatModule_sendMessage'+ seperator + username + seperator + message);\n
		}\n\n

		function changeUsername(newUsername) {\n
			username = newUsername;\n
			send("chatModule_changeUsername" + seperator + username);\n
		}\n\n

	 	function displayMessage(messageOwner, messageText, origin) {\n
	 		var messageContainer = document.getElementById('messageContainer');\n\n

			// Setup of the container which holds all elements of this message\n
			var completeMessage = document.createElement("div");\n
			completeMessage.className = origin + "CompleteMessage";\n\n

			// Create Username Container and fill with information\n
			var message_User = document.createElement("div");\n
			message_User.className = origin + "Username";\n
			message_User.innerHTML = messageOwner;\n
			var id = "message_" + messageIndex;\n\n

			// Create Content Container and fill with information\n
			var message_Content = "<div id ='"+ id + "' onclick='copyMessage(" + id + ");' class='" + origin + "Message " + origin + "Triangle'>" + messageText + "</div>";\n
			// Add the containers to complete the message and add to messageContainer\n
			completeMessage.appendChild(message_User);\n
			completeMessage.innerHTML += message_Content;\n
			messageContainer.appendChild(completeMessage);\n
			messageContainer.innerHTML += "<br>";\n\n

			// Scrolls the view down to the newest message\n
			messageContainer.scrollTop = messageContainer.scrollHeight;\n\n

			messageIndex ++;\n
	 	}\n\n

		function handleMessage() {\n
			var inputField = document.getElementById("messageContent");\n
			if(!isEmptyOrSpaces(inputField.value)) {\n
				var messageContent = inputField.value;\n
				displayMessage(username, messageContent, "self");\n
				sendMessage(messageContent);\n
			}\n
			inputField.value = "";\n
		}\n\n

		function isEmptyOrSpaces(str) {\n
    		return str === null || str.match(/^ *$/) !== null;\n
		}\n\n

		function receiveMessage(message) {\n
			var messageFragments = message.data.split(seperator);\n
			if(messageFragments[0] == "chatModule_chatMessage") {\n
				var user = messageFragments[1];\n
				var msg = messageFragments[2];\n
				displayMessage(user, msg, "remote");\n
			}\n
			if(messageFragments[0] == "chatModule_changeUsername") {\n
				var newUsername = messageFragments[1];\n
				changeUsername(newUsername);\n
			}\n
		}\n

		function copyMessage(message) {\n
		   var copyContainer = document.getElementById("messageContent");\n
		   var tempsave = copyContainer.value;\n
		   copyContainer.value = message.innerHTML;\n
		   copyContainer.select();\n
		   document.execCommand('copy');\n
		   copyContainer.value = tempsave;\n
		}\n\n

		var changeFormActive = false;\n\n

		function toggleUsernameChangeForm(isCancel) {\n
			var usernameEditIcon = document.getElementById("usernameEditIcon");\n
			var cancelEditIcon = document.getElementById("cancelEditIcon");\n
			var profile = document.getElementById("profile");\n
			var usernamePreview = document.getElementById("usernamePreview");\n\n

			if (changeFormActive) {\n
				if (isCancel) {\n
					usernamePreview.innerHTML = username;\n
				} else {\n
					var newUsername = usernamePreview.innerHTML;\n
					changeUsername(newUsername);\n
				}\n
				usernamePreview.contentEditable = "false";\n
				cancelEditIcon.style.display = "none";\n
				changeFormActive = false;\n
				usernameEditIcon.className = "fa far fa-solid fa-pencil";\n
			} else {\n
				usernameEditIcon.className = "fa far fa-solid fa-download";\n
				usernamePreview.contentEditable = "true";\n
				usernamePreview.focus();\n
				cancelEditIcon.style.display = "inline-block";\n
				changeFormActive = true;\n
			}\n
		}\n\n

		function sendDummy() {\n
			send("chatModule_chatMessage" + seperator + "Dummy" + seperator + "This is a test !");\n
		}\n

		function sendDebug(debugMessage) {\n
			send("chatModule_debugMessage" + seperator + debugMessage);\n
		}\n

		var muted = true;\n
		function toggleMute() {\n
			var mikeIcon = document.getElementById("mikeIcon");\n
            muted = !muted;\n
            if ( muted) mikeIcon.className = "fa fa-microphone-slash";\n
            if (!muted) mikeIcon.className = "fa fa-microphone";\n
			send("setMute|" + muted);\n
		}\n
		</script>\n
	</body>\n
</html>
);
