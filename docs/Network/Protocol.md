# Network Protocol

## Server Messages

#### `ServerMessage::PlayerId`
* Sent immediately after a client establishes a connection with the server

#### `ServerMessage::PlayerJoined`
* Broadcasted whenever a player joins a game lobby to all players but the player who joined

#### `ServerMessage::PlayerLeft`
* Broadcasted whenever a player intentionally leaves a game to all other players

#### `ServerMessage::LobbyOwner`
* Sent immediately to a player after that player joins the game lobby

#### `ServerMessage::OwnerLeft`
* Broadcasted to all players after the owner of the game lobby leaves

## Client Messages

#### `ClientMessage::InitServer`
* Sent immediately after a client launches a new server process by creating a game lobby

#### `ClientMessage::JoinServer`
* Sent whenever a player joins a lobby, just after connecting to the server in question
* Server responds with a `ServerMessage::LobbyOwner` message and by broadcasting a `ServerMessage::PlayerJoined` message to all _other_ players

#### `ClientMessage::LeaveGame`
* Sent whenever a player intentionally leaves a game
* Server responds by broadcasting a `ServerMessage::PlayerLeft` message to all _other_ players

