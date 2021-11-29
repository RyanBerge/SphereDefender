# Network Protocol

## Client Messages

#### `ClientMessage::InitLobby`
* Sent immediately after a client launches a new server process by creating a game lobby
* Server responds with a `ServerMessage::PlayerId` message
* `[playername:string]`

#### `ClientMessage::JoinLobby`
* Sent whenever a player joins a lobby, just after connecting to the server in question
* Server responds with a `ServerMessage::PlayersInLobby` message and by broadcasting a `ServerMessage::PlayerJoined` message to all _other_ players
* `[playername:string]`

#### `ClientMessage::StartGame`
* Sent when the owner of a lobby begins a game
* Server responds by broadcasting a `ServerMessage::StartGame` message to all players

#### `ClientMessage::LoadingComplete`
* Sent after the server starts a game when the game environment is finished loading
* Server responds after all players have finished loading with a `ServerMessage::AllPlayersLoaded` message

#### `ClientMessage::LeaveGame`
* Sent whenever a player intentionally leaves a game
* Server responds by broadcasting a `ServerMessage::PlayerLeft` message to all _other_ players

#### `ClientMessage::PlayerStateChange`
* Sent whenever the player state changes
* `[velocityflag:1]`

#### `ClientMessage::StartAction`
* Sent whenever the player takes an action the server needs to know
* `[actionflags:1][attackangle:2][...]`

## Server Messages

#### `ServerMessage::PlayerId`
* Sent immediately after a client establishes a connection with the server
* `[playerid:2]`

#### `ServerMessage::PlayerJoined`
* Broadcasted whenever a player joins a game lobby to all players but the player who joined
* * `[playerid:2][playername:string]`

#### `ServerMessage::PlayerLeft`
* Broadcasted whenever a player intentionally leaves a game to all other players
* `[playerid:2]`

#### `ServerMessage::OwnerLeft`
* Broadcasted to all other players after the owner of the game lobby leaves

#### `ServerMessage::PlayersInLobby`
* Sent immediately to a player after that player joins the game lobby
* The unique Player Id is sent first, then all players are listed, starting with the lobby owner
* `[playerid:2][numplayers:1][lobbyownerid:2][lobbyownername:string][playerid:2][playername:string][...][...]`

#### `ServerMessage::StartGame`
* Broadcast to all players after the owner of the game lobby starts the game
* All players respond with a `ClientMessage::LoadingComplete` message

#### `ServerMessage::AllPlayersLoaded`
* Sent after a game is started once the server has received a `ClientMessage::LoadingComplete` message from all players
* `[spawnpos:4][spawnpox:4]`

#### `ServerMessage::PlayerStates`
* Sent every frame
* `[numstates:1][playerid:2][posx:4][posy:4][playerid:2][posx:4][posy:4][...][...][...]`

#### `ServerMessage::PlayerStartAction`
* Broadcasted whenever a player starts an action
* `[playerid:2][actionflags:1][attackangle:2][...]`

#### `ServerMessage::EnemyStartAction`
* Sent whenever an enemy begins an action
* `[enemyid:2][actionflags:1][attackvector:8]`

#### `ServerMessage::RegionInfo`
* Sent when a region is created
* `[convoyposition:8][convoyorientation:1][numenemies:2][id:2][type:1][position:8][health:1][...]`

#### `ServerMessage::EnemyUpdate`
* Broadcasted every frame
* `[numenemies:2][id:2][position:8][health:1][...]`
