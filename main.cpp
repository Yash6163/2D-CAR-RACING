#include <SFML/Graphics.hpp>
/*
sf::RenderWindow – for creating and managing the game window.
	1.sf::Texture, sf::Sprite – for loading and displaying images.
	2.sf::Text, sf::Font – for rendering text on the screen.
	3.sf::Color – for coloring sprites and text.
    4.sf::FloatRect, sf::Vector2f – for positioning and collision detection.
*/
#include <SFML/System.hpp>
/*
Provides system utilities like:
	1.sf::Clock, sf::Time – for handling time and frame durations.
	2.Basic types like sf::Vector2f used for 2D positions and movement.
	3.Useful for timing game events (e.g., controlling movement speed based on frame time).
*/
#include <SFML/Window.hpp>
/*
Handles low-level windowing and input functionality.
	1.Provides:
	2.Input detection (sf::Keyboard, sf::Mouse).
	3.Window events (sf::Event) – used for processing user input (e.g., closing window, pressing keys).
*/
#include <iostream>
#include <vector>//storess multiple obj
#include <random>
/*        
Provides tools for random number generation.
Used in:
	1.std::random_device, std::mt19937 – seed and engine for generating random numbers.
	2.std::uniform_real_distribution – to randomize rock positions.
*/

int main() {
    // <-------------------START----------------------->
    //creating the main window with resolution 800x600 and it is titled "2D-RACING GAME"-
    sf::RenderWindow window(sf::VideoMode({800, 600}), "2D Racing Game");
    window.setFramerateLimit(60); //->Limit frame rate to 60 FPS (control game speed and smoothness)

    // Loading the first file - "road.png" in the cpp folder(where main.cpp is present)
    sf::Texture roadTexture;
    if (!roadTexture.loadFromFile("road.png")) {
        std::cerr << "Failed to load road texture" << std::endl;
        return -1; //(FAILSAFE) Exit if road texture cannot be loaded
    }

    // INFINITELYn loading the roads...yay
    sf::Sprite road1(roadTexture);
    sf::Sprite road2(roadTexture);

    // scale the road sprites to fit the game window-
    float scaleX = window.getSize().x / static_cast<float>(roadTexture.getSize().x);
    float scaleY = window.getSize().y / static_cast<float>(roadTexture.getSize().y);
    road1.setScale({scaleX, scaleY});
    road2.setScale({scaleX, scaleY});
    // positioning the second road sprite just above the first one to create seamless scrolling--
    road2.setPosition({0, -static_cast<float>(window.getSize().y)});

    // loading the rock.png used as obstacles--
    sf::Texture rockTexture;
    if (!rockTexture.loadFromFile("rock.png")) {
        std::cerr << "Failed to load rock texture" << std::endl;
        return -1; // Exit if rock texture cannot be loaded
    }

    //->>loading the car texture which the player will control
    sf::Texture carTexture;
    if (!carTexture.loadFromFile("car.png")) {
        std::cerr << "Failed to load car texture" << std::endl;
        return -1; // Exit if car texture cannot be loaded
    }
    //IMPROVEMENT MADE AFTER SEVERAL TRIES--
    // create the car sprite and scale it down to 50% of original size
    sf::Sprite carSprite(carTexture);
    float carScale = 0.5f;
    carSprite.setScale({carScale, carScale});

    // calculate initial position of the car at the bottom center of the window
    //so that the game is started from an initial position.
    float carStartX = (window.getSize().x - carTexture.getSize().x * carScale) / 2;
    float carStartY = window.getSize().y - carTexture.getSize().y * carScale - 20; //(fixed limit) 20 pixels from bottom
    carSprite.setPosition({carStartX, carStartY});

    //<vector> comes into play here ----
    // vector to hold multiple rock sprites representing obstacles on the road
    std::vector<sf::Sprite> rockSprites;

    // scale factor for rocks to reduce their size
    float rockScale = 0.6f; //one can adjust this value to change rock size
    float rockWidth = rockTexture.getSize().x * rockScale;
    float rockHeight = rockTexture.getSize().y * rockScale;

    // this sets up random number generators for rock positions
    std::random_device rd;  // Obtain a random seed
    std::mt19937 gen(rd()); // Mersenne Twister random number engine
    //high-quality pseudorandom number generator commonly used in C++(SOURCE- OSDC)


    // uniform distribution for X positions within the window width minus rock width
    std::uniform_real_distribution<float> distX(0, window.getSize().x - rockWidth);
    //Example: If your window width is 800 and rock width is 60, this will produce X positions between 0 and 740.

    // uniform distribution for Y positions above the visible window (negative range)
    std::uniform_real_distribution<float> distY(-window.getSize().y, -rockHeight);
    //Example: If the window height is 600 and rock height is 60, this produces values from -600 to -60.

    // creates initial rock sprites positioned randomly above the window for scrolling down
    for (int i = 0; i < 2; ++i) {
        sf::Sprite rockSprite(rockTexture);
        rockSprite.setScale({rockScale, rockScale}); // Apply scaling to rock sprite
        float rockX = distX(gen); // Random X position
        float rockY = distY(gen); // Random Y position above window
        rockSprite.setPosition({rockX, rockY});
        std::cout << "Rock position: " << rockX << ", " << rockY << std::endl;
        rockSprites.push_back(rockSprite);
    }

    // this addss a test rock sprite at a fixed position for visibility testing
    sf::Sprite testRock(rockTexture);
    testRock.setScale({rockScale, rockScale});
    testRock.setPosition({400.f, 300.f});
    rockSprites.push_back(testRock);

    // this sets the scrolling speed of the road and rocks (pixels per second)
    float scrollSpeed = 200.0f;

    // this efines the movement speed of the car controlled by the player
    float carSpeed = 300.0f; // pixels per second

    // Clock object to measure elapsed time between frames for smooth movement
    sf::Clock clock;

    // Game state variables
    bool gameOver = false;

    // Load font for displaying text on screen
    sf::Font font;
    std::cout << "Trying to load font...\n";
    if (!font.openFromFile("roboto.ttf")) {
        std::cerr << "Failed to load 'roboto.ttf'. Trying fallback font.\n";
        // Attempt to load fallback system font if custom font fails
        if (!font.openFromFile("/System/Library/Fonts/SFNS.ttf")) {
            std::cerr << "Failed to load fallback system font as well.\n";
            return -1; // Exit if no font can be loaded
        } else {
            std::cout << "Loaded fallback system font successfully.\n";
        }
    } else {
        std::cout << "Font loaded successfully.\n";
    }

    // Text object to display "Game Over" message
    sf::Text gameOverText(font, "                  ~~GAME OVER~~\n", 30);
    gameOverText.setFillColor(sf::Color::Magenta);
    gameOverText.setPosition(sf::Vector2f(150.f, 250.f));


    //---------------MAIN GAME LOOP WHERE EVERYTHING GETS TOGETHER----------------
    // Main game loop runs while the window is open
    while (window.isOpen()) {
        // Measure time elapsed since last frame for smooth movement calculations
        sf::Time deltaTime = clock.restart();


        // Process all pending events in the window's event queue
        auto eventOpt = window.pollEvent();
        while (eventOpt) {
            const auto& event = *eventOpt;
            // Close window event
            if (event.is<sf::Event::Closed>()) {
                window.close();
            }
            // If game is over, check for restart or exit key presses
            if (gameOver) {
                if (event.is<sf::Event::KeyPressed>()) {
                    const auto* keyEvent = event.getIf<sf::Event::KeyPressed>();
                    if (keyEvent) {
                        auto key = keyEvent->code;
                        if (key == sf::Keyboard::Key::R) {
                            // Restart the game: reset car position, reposition rocks randomly, reset score and game state
                            carSprite.setPosition({carStartX, carStartY});
                            for (auto& rock : rockSprites) {
                                float rockX = distX(gen);
                                float rockY = distY(gen);
                                rock.setPosition({rockX, rockY});
                            }
                            gameOver = false;
                        } else if (key == sf::Keyboard::Key::Escape) {
                            // Exit game if Escape key is pressed
                            window.close();
                        }
                    }
                }
            }
            eventOpt = window.pollEvent();
        }

        // Calculate the amount of movement for this frame based on scroll speed and elapsed time
        float movement = scrollSpeed * deltaTime.asSeconds();

        // Move both road sprites downwards to simulate road scrolling
        road1.move({0, movement});
        road2.move({0, movement});

        // Reset road sprite positions when they move below the window to create continuous scrolling effect
        if (road1.getPosition().y >= window.getSize().y) {
            road1.setPosition({0, road2.getPosition().y - window.getSize().y});
        }
        if (road2.getPosition().y >= window.getSize().y) {
            road2.setPosition({0, road1.getPosition().y - window.getSize().y});
        }

        // Move each rock obstacle downwards
        for (auto& rock : rockSprites) {
            rock.move({0, movement});

            // If a rock moves below the window, respawn it just above the screen at a new random X position
            if (rock.getPosition().y >= window.getSize().y) {
                float rockX = distX(gen);
                float rockY = -rockHeight; // Position just above the visible window
                rock.setPosition({rockX, rockY});
                std::cout << "Respawned rock at: (" << rockX << ", " << rockY << ")\n"; // Debug output
            }
        }

        //--------KEYBOARD CONTROLS-----------
        // Handle keyboard input for car movement
        sf::Vector2f carMove(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            carMove.x -= carSpeed * deltaTime.asSeconds(); // Move left
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            carMove.x += carSpeed * deltaTime.asSeconds(); // Move right
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            carMove.y -= carSpeed * deltaTime.asSeconds(); // Move up
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            carMove.y += carSpeed * deltaTime.asSeconds(); // Move down
        }

        //(THE BORDER) Define the boundaries of the road where the car is allowed to move
        sf::FloatRect roadBounds(
            sf::Vector2f(window.getSize().x * 0.25f, 0), // Left boundary at 25% of window width
            sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y) // Road width is 50% of window width
        );

        // Calculate the next position of the car after applying movement
        sf::FloatRect nextPos = carSprite.getGlobalBounds();
        sf::Vector2f nextPosPos = nextPos.position;
        sf::Vector2f nextPosSize = nextPos.size;
        nextPosPos.x += carMove.x;
        nextPosPos.y += carMove.y;

        // Check if the center of the car's next position is within the road boundaries before moving
        if (roadBounds.contains({
                nextPosPos.x + nextPosSize.x / 2,
                nextPosPos.y + nextPosSize.y / 2 })) {
            carSprite.move(carMove); // Move car if within bounds
        }

        // Check for collisions between the car and rocks
        for (const auto& rock : rockSprites) {
            sf::FloatRect carBounds = carSprite.getGlobalBounds();
            auto rockBounds = rock.getGlobalBounds();
            auto rockPos = rockBounds.position;
            auto rockSize = rockBounds.size;
            float marginX = 10.f; // Margin to shrink rock collision box for better gameplay
            float marginY = 10.f;
            // Adjust rock bounds to create a smaller collision box inside the rock sprite
            sf::FloatRect adjustedRockBounds(
                {rockPos.x + marginX, rockPos.y + marginY},
                {rockSize.x - 2 * marginX, rockSize.y - 2 * marginY}
            );

            auto carPos = carBounds.position;
            auto carSize = carBounds.size;
            if (adjustedRockBounds.contains({
                    carPos.x + carSize.x / 2,
                    carPos.y + carSize.y / 2 })) {
            // 'intersection' now contains the overlapping area
                // Set game over state
                gameOver = true;

                // Immediately draw the game over screen and updated game state
                window.clear();
                window.draw(road1);
                window.draw(road2);
                for (const auto& r : rockSprites) {
                    window.draw(r);
                }
                window.draw(carSprite);
                window.draw(gameOverText);
                window.display();
                sf::sleep(sf::seconds(5)); // Wait for 5 seconds
                window.close();            // Then close the game
                break; // Exit collision checking loop after game over
            }
        }

        // Clear the window to prepare for new frame drawing
        window.clear();

        // Draw the scrolling road sprites
        window.draw(road1);
        window.draw(road2);

        // Draw all rock obstacles
        for (const auto& rock : rockSprites) {
            window.draw(rock);
        }

        // Draw the player's car sprite
        window.draw(carSprite);

        // ----> If game is over, draw the game over message on top
        if (gameOver) {
            window.draw(gameOverText);
        }

        // ----> Display the rendered frame on the screen
        window.display();
    }

    return 0;
}