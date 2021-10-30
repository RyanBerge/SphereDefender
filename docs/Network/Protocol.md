# Network Protocol

## Server Messages

#### `ServerMessage::PlayerId`
* Sent immediately after a client establishes a connection with the server

#### `ServerMessage::PlayerJoined`
* Broadcasted whenever a player joins a game lobby to all players but the player who joined

#### `ServerMessage::PlayerLeft`
* Broadcasted whenever a player intentionally leaves a game to all other players

#### `ServerMessage::PlayersInLobby`
* Sent immediately to a player after that player joins the game lobby
* All players are listed, starting with the lobby owner

#### `ServerMessage::OwnerLeft`
* Broadcasted to all players after the owner of the game lobby leaves

#### `ServerMessage::StartGame`
* Broadcast to all players after the owner of the game lobby starts the game
* All players respond with a `ClientMessage::LoadingComplete` message

#### `ServerMessage::AllPlayersLoaded`
* Sent after a game is started once the server has received a `ClientMessage::LoadingComplete` message from all players

## Client Messages

#### `ClientMessage::InitServer`
* Sent immediately after a client launches a new server process by creating a game lobby

#### `ClientMessage::JoinServer`
* Sent whenever a player joins a lobby, just after connecting to the server in question
* Server responds with a `ServerMessage::LobbyOwner` message and by broadcasting a `ServerMessage::PlayerJoined` message to all _other_ players

#### `ClientMessage::StartGame`
* Sent when the owner of a lobby begins a game
* Server responds by broadcasting a `ServerMessage::StartGame` message to all players

#### `ClientMessage::LoadingComplete`
* Sent after the server starts a game when the game environment is finished loading
* Server responds after all players have finished loading with a `ServerMessage::AllPlayersLoaded` message

#### `ClientMessage::LeaveGame`
* Sent whenever a player intentionally leaves a game
* Server responds by broadcasting a `ServerMessage::PlayerLeft` message to all _other_ players

