#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace {
    const sf::Vector2f DEFAULT_RATIO = {1200, 800};
}

void ResizeWindow(sf::RenderWindow& window, sf::Vector2f ratio);

int main()
{
    sf::RenderWindow window(sf::VideoMode(DEFAULT_RATIO.x, DEFAULT_RATIO.y), "SphereDefender");

    sf::Sprite splash_screen;
    sf::Texture splash_texture;

    splash_texture.loadFromFile("assets/SplashScreen.png");
    splash_screen.setTexture(splash_texture);

    while (window.isOpen())
    {
        sf::Event event;

        window.clear(sf::Color::Black);

        window.draw(splash_screen);

        window.display();

        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window.close();
                }
                break;
                case sf::Event::Resized:
                {
                    sf::Vector2f ratio = sf::Vector2f(event.size.width, event.size.height);
                    ResizeWindow(window, ratio);
                }
                break;
                default:
                {
                    break;
                }
            }
        }
    }
}

void ResizeWindow(sf::RenderWindow& window, sf::Vector2f ratio)
{
    float aspect_ratio = DEFAULT_RATIO.x / DEFAULT_RATIO.y;
    float window_ratio = (ratio.x / ratio.y);

    float viewport_width = 1;
    float viewport_height = 1;
    float viewport_x = 0;
    float viewport_y = 0;

    if (window_ratio > aspect_ratio)
    {
        viewport_width = aspect_ratio / window_ratio;
        viewport_x = (1 - viewport_width) / 2;
    }
    else if (window_ratio < aspect_ratio)
    {
        viewport_height = window_ratio / aspect_ratio;
        viewport_y = (1 - viewport_height) / 2;
    }

    sf::FloatRect view_rect{0, 0, 1200, 800};
    view_rect.left = window.getView().getCenter().x - window.getView().getSize().x / 2;
    view_rect.top = window.getView().getCenter().y - window.getView().getSize().y / 2;

    sf::View view(view_rect);
    view.setViewport(sf::FloatRect(viewport_x, viewport_y, viewport_width, viewport_height));
    window.setView(view);
}
