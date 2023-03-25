#include <SFML/Graphics.hpp>

using namespace sf;

int main()
{
    RenderWindow window(VideoMode(800, 600), "ZombieGame");

    Event event;

    while (window.isOpen()) {

        while (window.pollEvent(event)) {

            if (event.type == Event::Closed) {

                window.close();
            }
        }
    }

    return 0;
}