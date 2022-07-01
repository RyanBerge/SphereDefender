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

#### `ClientMessage::ChangePlayerProperty`
* Sent whenever a player changes a property while in the lobby
* `[class:1]`

#### `ClientMessage::StartGame`
* Sent when the owner of a lobby begins a game
* Server responds by broadcasting a `ServerMessage::StartGame` message to all players

#### `ClientMessage::LoadingComplete`
* Sent after the server starts a game or when changing regions, when the game environment is finished loading
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

#### `ClientMessage::UseItem`
* Sent when a player uses an item

#### `ClientMessage::SwapItem`
* Sent when a player swaps an item from the stash
* `[itemindex:1]`

#### `ClientMessage::Console`
* Sent when a player opens or closes the console
* `[activate:1]`

#### `ClientMessage::ChangeRegion`
* Sent when a player interacts with the console to move regions
* `[regionid:2]`

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
* `[playerid:2][numplayers:1][lobbyownerid:2][lobbyownername:string][ownerclass:1][playerid:2][playername:string][playerclass:1][...][...]`

#### `ClientMessage::ChangePlayerProperty`
* Broadcasted to all other players after a player changes a property while in the lobby
* `[playerid:2][class:1]`

#### `ServerMessage::StartGame`
* Broadcast to all players after the owner of the game lobby starts the game
* All players respond with a `ClientMessage::LoadingComplete` message

#### `ServerMessage::AllPlayersLoaded`
* Sent after a game is started once the server has received a `ClientMessage::LoadingComplete` message from all players
* `[spawnpos:4][spawnpox:4]`

#### `ServerMessage::SetPaused`
* Broadcasted whenever the game needs to be paused or resumed
* `[pause:1]`

#### `ServerMessage::PlayerStartAction`
* Broadcasted whenever a player starts an action
* `[playerid:2][actionflags:1][attackangle:2][...]`

#### `ServerMessage::EnemyChangeAction`
* Sent whenever an enemy begins an action
* `[enemyid:2][actionflags:1][attackvector:8]`

#### `ServerMessage::ChangeItem`
* Sent to a player when their item changes
* `[itemtype:1]`

#### `ServerMessage::PlayerStates`
* Sent every frame
* `[numstates:1][playerid:2][position:8][health:1][...]`

#### `ServerMessage::EnemyUpdate`
* Broadcasted every frame
* `[numenemies:2][id:2][position:8][health:1][charge:4][...]`

#### `ServerMessage::BatteryUpdate`
* Broadcasted every frame
* `[batterylevel:4]`

#### `ServerMessage::ProjectileUpdate`
* Broadcasted every frame
* `[numprojectiles:2][id:2][position:8]`

#### `ServerMessage::ChangeRegion`
* Broadcasted when a region change happens
* `[regionid:2]`

#### `ServerMessage::UpdateStash`
* Broadcasted when the stash changes
* `[itemtype:1][itemtype:1][...]`
