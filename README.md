# Chess-Competition

This is a C++ chess competition base repository. Here we are going to host the frontend, backend and the simulator of the competition.

## Main activity

You have to code a chess engine from the ground up. You will receive a string as input following [FEN](https://www.chess.com/terms/fen-chess) notation and the output should be the move as [LAN](https://www.chessprogramming.org/Algebraic_Chess_Notation). 

## Rules

- Implement your chess engine only inside the chess-library folder in C++;
- Create .h and .cpp files inside chess-library; 
- Obey the interface specified on chess-library;
- You might want to test your code via terminal via chess-cli, or chess-gui;
- Merge requests are welcome;
- When you submit your code, you should zip only the contents of the chess-library folder and send it to the system;
- Do not use sub-folders inside the chess-library folder, it will break my automation;

## Folder structure

- chess-library: Here you will implement your chess engine;
- chess-cli: Here you will find the chess-cli code;
- chess-gui: Here you will find the chess-gui code;
- frontend: Here you will find the web frontend code;
- backend: Here you will find the web backend code;

## How the competition will work

- A tournament will run every day;
- The tournament consists of making all players playing against each other on both sides of the board;
- You may use up to 16GB ram and 12 cores of CPU;
- Each turn should take less than 10 seconds;
- You are not allowed to use any external library;
- You can use AI assisted tools, but you have to state it on your code, and you will receive a penalty on your final score of 20%;
- You may work in teams of up to 2 people;
- Your username should not match your real name by any means in order do follow FERPA compliance;