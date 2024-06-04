#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <string> // Added for std::to_string()

std::string resourcesDir()
{
    return "resources/";
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const float pi = 3.14159f;
    const float gameWidth = 800;
    const float gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 10.f;

    // Additional constants for ball speed
    float initialBallSpeed = 400.f;
    float ballAcceleration = 10.f;
    float currentBallSpeed = initialBallSpeed;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight), 32), "SFML Tennis",
        sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile(resourcesDir() + "ball.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);

    // Create the left paddle
    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    leftPaddle.setOutlineThickness(3);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize / 2.f);

    // Create the right paddle
    sf::RectangleShape rightPaddle;
    rightPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    rightPaddle.setOutlineThickness(3);
    rightPaddle.setOutlineColor(sf::Color::Black);
    rightPaddle.setFillColor(sf::Color(200, 100, 100));
    rightPaddle.setOrigin(paddleSize / 2.f);

    // Create the ball
    sf::CircleShape ball;
    ball.setRadius(ballRadius - 3);
    ball.setOutlineThickness(2);
    ball.setOutlineColor(sf::Color::Black);
    ball.setFillColor(sf::Color::White);
    ball.setOrigin(ballRadius / 2, ballRadius / 2);

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile(resourcesDir() + "tuffy.ttf"))
        return EXIT_FAILURE;

    // Initialize the pause message
    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 200.f);
    pauseMessage.setFillColor(sf::Color::White);

    // Define the paddles properties
    sf::Clock AITimer;
    const sf::Time AITime = sf::seconds(0.1f);
    const float paddleSpeed = 400.f;
    float rightPaddleSpeed = 0.f;
    float ballAngle = 0.f; // to be changed later

    // Define scores and winning score
    int leftScore = 0;
    int rightScore = 0;
    const int winningScore = 5;

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(40);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition((gameWidth - scoreText.getLocalBounds().width) / 2.f, 10.f);

    sf::Clock clock;
    bool isPlaying = false;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }

            if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
                (event.type == sf::Event::TouchBegan))
            {
                if (!isPlaying)
                {
                    isPlaying = true;
                    clock.restart();

                    leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
                    rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
                    ball.setPosition(gameWidth / 2.f, gameHeight / 2.f);

                    do
                    {
                        ballAngle = static_cast<float>(std::rand() % 360) * 2.f * pi / 360.f;
                    } while (std::abs(std::cos(ballAngle)) < 0.7f);

                    // Reset ball speed
                    currentBallSpeed = initialBallSpeed;
                }
            }

            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(gameWidth, gameHeight);
                view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
                window.setView(view);
            }
        }

        if (isPlaying)
        {
            float deltaTime = clock.restart().asSeconds();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
                (leftPaddle.getPosition().y - paddleSize.y / 2 > 5.f))
            {
                leftPaddle.move(0.f, -paddleSpeed * deltaTime);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
                (leftPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f))
            {
                leftPaddle.move(0.f, paddleSpeed * deltaTime);
            }

            if (sf::Touch::isDown(0))
            {
                sf::Vector2i pos = sf::Touch::getPosition(0);
                sf::Vector2f mappedPos = window.mapPixelToCoords(pos);
                leftPaddle.setPosition(leftPaddle.getPosition().x, mappedPos.y);
            }

            if (((rightPaddleSpeed < 0.f) && (rightPaddle.getPosition().y - paddleSize.y / 2 > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rightPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f)))
            {
                rightPaddle.move(0.f, rightPaddleSpeed * deltaTime);
            }

            if (AITimer.getElapsedTime() > AITime)
            {
                AITimer.restart();
                if (ball.getPosition().y + ballRadius > rightPaddle.getPosition().y + paddleSize.y / 2)
                    rightPaddleSpeed = paddleSpeed;
                else if (ball.getPosition().y - ballRadius < rightPaddle.getPosition().y - paddleSize.y / 2)
                    rightPaddleSpeed = -paddleSpeed;
                else
                    rightPaddleSpeed = 0.f;
            }

            // Increase ball speed
            currentBallSpeed += ballAcceleration * deltaTime;

            // Move the ball
            float factor = currentBallSpeed * deltaTime;
            ball.move(std::cos(ballAngle) * factor, std::sin(ballAngle) * factor);

            const std::string inputString = "Press space to restart or\nescape to exit.";

            if (ball.getPosition().x - ballRadius < 0.f)
            {
                isPlaying = false;
                rightScore++;
                if (rightScore >= winningScore) {
                    pauseMessage.setString("You Won!\n\n" + inputString);
                }
                else {
                    pauseMessage.setString("Right Player Scored!\n\n" + inputString);
                }
            }
            if (ball.getPosition().x + ballRadius > gameWidth)
            {
                isPlaying = false;
                leftScore++;
                if (leftScore >= winningScore) {
                    pauseMessage.setString("You Won!\n\n" + inputString);
                }
                else {
                    pauseMessage.setString("Left Player Scored!\n\n" + inputString);
                }
            }
            if (ball.getPosition().y - ballRadius < 0.f)
            {
                ballSound.play();
                ballAngle = -ballAngle;
                ball.setPosition(ball.getPosition().x, ballRadius + 0.1f);
            }
            if (ball.getPosition().y + ballRadius > gameHeight)
            {
                ballSound.play();
                ballAngle = -ballAngle;
                ball.setPosition(ball.getPosition().x, gameHeight - ballRadius - 0.1f);
            }

            if (ball.getPosition().x - ballRadius < leftPaddle.getPosition().x + paddleSize.x / 2 &&
                ball.getPosition().x - ballRadius > leftPaddle.getPosition().x &&
                ball.getPosition().y + ballRadius >= leftPaddle.getPosition().y - paddleSize.y / 2 &&
                ball.getPosition().y - ballRadius <= leftPaddle.getPosition().y + paddleSize.y / 2)
            {
                float relativeIntersectY = leftPaddle.getPosition().y - ball.getPosition().y;
                float normalizedRelativeIntersectionY = (relativeIntersectY / (paddleSize.y / 2));
                ballAngle = normalizedRelativeIntersectionY * (pi / 4); // Max bounce angle 45 degrees
                ballSound.play();
                ball.setPosition(leftPaddle.getPosition().x + ballRadius + paddleSize.x / 2 + 0.1f, ball.getPosition().y);
            }

            if (ball.getPosition().x + ballRadius > rightPaddle.getPosition().x - paddleSize.x / 2 &&
                ball.getPosition().x + ballRadius < rightPaddle.getPosition().x &&
                ball.getPosition().y + ballRadius >= rightPaddle.getPosition().y - paddleSize.y / 2 &&
                ball.getPosition().y - ballRadius <= rightPaddle.getPosition().y + paddleSize.y / 2)
            {
                float relativeIntersectY = rightPaddle.getPosition().y - ball.getPosition().y;
                float normalizedRelativeIntersectionY = (relativeIntersectY / (paddleSize.y / 2));
                ballAngle = pi - normalizedRelativeIntersectionY * (pi / 4); // Max bounce angle 45 degrees
                ballSound.play();
                ball.setPosition(rightPaddle.getPosition().x - ballRadius - paddleSize.x / 2 - 0.1f, ball.getPosition().y);
            }
        }

        window.clear(sf::Color(50, 50, 50));

        // Update and draw scores
        scoreText.setString(std::to_string(leftScore) + " - " + std::to_string(rightScore));
        scoreText.setPosition((gameWidth - scoreText.getLocalBounds().width) / 2.f, 10.f);
        window.draw(scoreText);

        if (isPlaying)
        {
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            window.draw(ball);
        }
        else
        {
            window.draw(pauseMessage);
        }

        window.display();
    }

    return EXIT_SUCCESS;
}
