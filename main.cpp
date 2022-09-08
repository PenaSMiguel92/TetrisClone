#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <random>

#define PI_VALUE 3.14159265

//class obtained from stack overflow
class Randomer {
    //random seed by default
    std::mt19937 gen_;
    std::uniform_int_distribution<size_t> dist_;
public:
    /* ... some convenient constructors... */
    Randomer(size_t min, size_t max, unsigned int seed = std::random_device{}())
        : gen_ {seed}, dist_{min,max}{}
    void SetSeed(unsigned int seed) {
        gen_.seed(seed);
    }
    size_t operator()() {
        return dist_(gen_);
    }
};

class Map
{
public:
    sf::Vector2<int> mapSize;
    std::vector<int> mapGrid;
    sf::Vector2<int> mapOffset;
    sf::Vector2<int> mapDrawOffset;
    sf::Vector2<int> gridUnitSize = sf::Vector2<int>(32, 32); //grid unit size in pixels;

    Map(sf::Vector2<int> _mapSize, sf::Vector2<int> _mapOffset, sf::Vector2<int> _mapDrawOffset)
    {
        mapSize = _mapSize;
        mapOffset = _mapOffset;
        mapDrawOffset = _mapDrawOffset;
        setupMap();
    }
    void setupMap()
    {
        for (int index = 0; index < mapSize.x * mapSize.y; index++)
        {
            mapGrid.push_back(0);
        }
    }
    bool testPosition(sf::Vector2<int> position)
    {
        if (position.x < 0 || position.x >= mapSize.x)
            return false;
        int index = (position.y * mapSize.x) + position.x;
        if (index < 0 || index >= mapSize.x * mapSize.y)
            return false;
        return (mapGrid[index] == 0);
    }
    void setAtPosition(int typeToUse, sf::Vector2<int> position, std::vector<std::vector<int>> structure)
    {
        int xLimit = structure[0].size();
        int yLimit = structure.size();
        for (int index = 0; index < xLimit * yLimit; index++)
        {
            int mx = index % xLimit;
            int my = floor(index / xLimit);
            if (structure[my][mx] == 1) 
            {
                int mapIndex = ((position.y + my) * mapSize.x) + (position.x+mx);
                mapGrid[mapIndex] = typeToUse+1;
            }
        }
    }
    void drawSelf(sf::RenderWindow &window)
    {
        sf::RectangleShape rectShapeA;
        rectShapeA.setSize(sf::Vector2f(gridUnitSize.x * (mapSize.x + mapOffset.x),
                                        gridUnitSize.y * (mapSize.y + mapOffset.y)));
        rectShapeA.setOutlineColor(sf::Color::Black);
        rectShapeA.setFillColor(sf::Color(200, 175, 215, 255));
        rectShapeA.setOutlineThickness(1);
        rectShapeA.setPosition(sf::Vector2f(mapDrawOffset.x, mapDrawOffset.y));
        window.draw(rectShapeA);
        int ignoredIndex = abs(mapOffset.y)*mapSize.x;
        sf::Color colorChoices[] = { sf::Color::Yellow,sf::Color::White, sf::Color::Red, sf::Color(255,135,0,255), sf::Color::Magenta, sf::Color::Blue, sf::Color::Green, sf::Color(100,100,100,255)};
        for (int index = 0; index < mapSize.x * mapSize.y; index++)
        {
            if ((mapGrid[index] == 0) || (index < ignoredIndex)) //ignore the indexes that are above the drawing area, ensures tetranimo arrives above screen area.
                continue;
            int mx = index % mapSize.x;
            int my = floor(index / mapSize.x);
            int colorIndex;
            if (mapGrid[index] > 0)
                colorIndex = mapGrid[index] - 1;
            if (mapGrid[index] < 0)
                colorIndex = 7;
            sf::RectangleShape rectShape;
            rectShape.setSize(sf::Vector2f(gridUnitSize.x,gridUnitSize.y));
            rectShape.setOutlineColor(sf::Color::Black);
            rectShape.setFillColor(colorChoices[colorIndex]);
            rectShape.setOutlineThickness(1);
            rectShape.setPosition((mx+mapOffset.x) * gridUnitSize.x + mapDrawOffset.x, (my+mapOffset.y) * gridUnitSize.y + mapDrawOffset.y);
            window.draw(rectShape);
        }
    }
    std::vector<int> checkForCompletedLines()
    {
        std::vector<int> fullRows;
        int count = 0;
        for (int index = (mapSize.x * mapSize.y) - 1; index >= 0; index--) 
        {
            int mx = index % mapSize.x;
            int my = floor(index / mapSize.x);
            if ((mx >= 0) && (mapGrid[index] > 0)) {
                count++;
            }
            if ((mx == 0) && (count < mapSize.x)) 
            {
                count = 0;
            } else if ((mx == 0) && (count == mapSize.x))
            {
                fullRows.push_back(my);
                //std::cout << "Row added: " << my << std::endl;
                count = 0;
            }
                
        }
        return fullRows;
    }
    void markForDeletion(std::vector<int> completeRows) 
    {
        for (int index = 0; index < completeRows.size();index++)
        {
            int my = completeRows[index];
            for (int mx = 0; mx < mapSize.x; mx++)
            {
                int mapIndex = (my * mapSize.x) + mx;
                mapGrid[mapIndex] = -1;
            }
        }
    }
    void deleteMarkedRows()
    {
        int count = 0; //count number of deleted values
        for (int index = (mapSize.x * mapSize.y) - 1; index >= 0; index--) 
        {
            if (mapGrid[index] == -1) 
            {
                mapGrid.erase(mapGrid.begin() + index);
                count++;
            }
        }
        for (int i = 0; i < count; i++) mapGrid.insert(mapGrid.begin(), 0);
    }
    void reset()
    {
        mapGrid.clear();
        for (int index = 0; index < mapSize.x * mapSize.y; index++)
        {
            mapGrid.push_back(0);
        }
    }
};

class Tetranimo
{
public:
    sf::Vector2<int> position = sf::Vector2<int>(0,0);
    int type = 0; //0 - square, 1 - line, 2 - s, 3 - z, 4 - t, 5 - rl, 6 - ll;
    int nextType;
    sf::Color currentColor = sf::Color::Yellow;
    std::vector<std::vector<int>> structure{
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    };
    std::vector<std::vector<int>> newStructure{
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    };
    std::vector<std::vector<int>> listOfStructures[7] = {
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0}
        },
        {
            {0,0,0,0},
            {0,0,1,1},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {0,0,1,0},
            {0,1,1,0},
            {0,0,0,0}
        }
    };
    std::vector<int> bagOfTypes{ 0, 1, 2, 3, 4, 5, 6 };
    float fallSpeed = 1.25;
    int getNextTetranimoType()
    {
        if (bagOfTypes.empty())
            bagOfTypes = { 0, 1, 2, 3, 4, 5, 6 };
        Randomer randomer{ 0,bagOfTypes.size()-1};
        int chosenTypeIndex = randomer();
        int chosenType = bagOfTypes[chosenTypeIndex];
        bagOfTypes.erase(bagOfTypes.begin()+chosenTypeIndex);
        return chosenType;
    }
    void setTetranimoType(int _type) 
    {
        type = _type;
        sf::Color colorChoices[] = { sf::Color::Yellow,sf::Color::White, sf::Color::Red, 
                                     sf::Color(255,135,0,255), sf::Color::Magenta, sf::Color::Blue, 
                                     sf::Color::Green, sf::Color(100,100,100,255) };
        currentColor = colorChoices[type];
        structure = listOfStructures[type];
    }
    bool updatePosition(int _x, int _y, Map &mainMap)
    {
        sf::Vector2<int> vector = sf::Vector2<int>(_x, _y);
        sf::Vector2<int> nextPosition = position + vector;
        bool okayToUpdate = true;
        for (int index = 0; index < structure.size() * structure[0].size(); index++)
        {
            int x = index % structure[0].size();
            int y = floor(index / structure[0].size());
            if (structure[y][x] == 1) {
                if (!mainMap.testPosition(nextPosition + sf::Vector2<int>(x,y))) 
                {
                    okayToUpdate = false;
                    break;
                }
                    
            }
        }
        if (okayToUpdate)
            position += vector;
        return okayToUpdate;
    }
    void rotateStructure(float angle, Map &mainMap)
    {
        bool okayToUpdate = true;
        int yLimit = structure.size();
        int xLimit = structure[0].size();
        float centerX = float(xLimit / 2)-0.5;
        float centerY = float(yLimit / 2)-0.5;
        std::vector<std::vector<int>> newStructure{
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        };
        for (int index = 0; index < yLimit*xLimit; index++) 
        {
            int x = index % xLimit;
            int y = floor(index / yLimit); //used for accessing structure elements
            float x_float = float(index % xLimit)-centerX; //floating point arithmetic
            float y_float = float(index / yLimit)-centerY;
            float xp_float = float(x_float * cos(angle) - y_float * sin(angle))+centerX;
            float yp_float = float(x_float * sin(angle) + y_float * cos(angle))+centerY;
            int xp = int(round(xp_float)); 
            int yp = int(round(yp_float)); //used for accessing new structure elements           
            newStructure[yp][xp] = structure[y][x];
            if (newStructure[yp][xp] == 1) {
                if (!mainMap.testPosition(position + sf::Vector2<int>(xp, yp)))
                {
                    okayToUpdate = false;
                    break;
                }
            }
            
        }
        if (okayToUpdate)
            structure = newStructure;
    }
    void drawSelf(sf::RenderWindow &window, Map &mapRef) 
    {
        int xLimit = structure[0].size();
        int yLimit = structure.size();
        for (int index = 0; index < xLimit*yLimit; index++) //Ensures O(n) complexity
        {
            int x = index % xLimit;
            int y = floor(index / xLimit);
            if (position.y + y < abs(mapRef.mapOffset.y))
                continue; //don't draw if above drawing area indicated by mapOffset.
            if (structure[y][x] == 1)
            {
                
                sf::RectangleShape rectShapeB;
                rectShapeB.setSize(sf::Vector2f(mapRef.gridUnitSize.x, mapRef.gridUnitSize.y));
                rectShapeB.setOutlineColor(sf::Color::Black);
                rectShapeB.setFillColor(currentColor);
                rectShapeB.setOutlineThickness(1);
                rectShapeB.setPosition(mapRef.gridUnitSize.x * (position.x + x + mapRef.mapOffset.x) + mapRef.mapDrawOffset.x, 
                                      mapRef.gridUnitSize.y * (position.y + y + mapRef.mapOffset.y) + mapRef.mapDrawOffset.y);
                window.draw(rectShapeB);
            } 
        }
    }
    void reset()
    {
        position = sf::Vector2<int>(4, 0);
        setTetranimoType(nextType);
        nextType = getNextTetranimoType();
        newStructure = listOfStructures[nextType];
    }
};

class UserInterface
{
public:
    sf::Font mainFont;
    UserInterface(std::string _fontFileName) 
    {
        loadFont(_fontFileName);
    }
    void loadFont(std::string fontFileName)
    {
        mainFont.loadFromFile(fontFileName);
    }
    void drawText(sf::RenderWindow &window, std::string text, sf::Vector2f position)
    {
        sf::Text textObj;
        textObj.setFont(mainFont);
        textObj.setFillColor(sf::Color::White);
        textObj.setCharacterSize(32);
        textObj.setString(text);
        textObj.setPosition(position);
        window.draw(textObj);
    }
    void drawNextTetranimo(sf::RenderWindow &window, int type, std::vector<std::vector<int>> structure, sf::Vector2f position,
                           sf::Vector2<int> gridUnitSize, sf::Color backgroundColor)
    {
        sf::Color colorChoices[] = { sf::Color::Yellow,sf::Color::White, sf::Color::Red, sf::Color(255,135,0,255), 
                                     sf::Color::Magenta, sf::Color::Blue, sf::Color::Green, sf::Color(100,100,100,255) };
        int xLimit = structure[0].size();
        int yLimit = structure.size();
        sf::RectangleShape rectShapeA;
        rectShapeA.setSize(sf::Vector2f(gridUnitSize.x * xLimit, gridUnitSize.y * yLimit));
        rectShapeA.setOutlineColor(sf::Color::Black);
        rectShapeA.setFillColor(backgroundColor);
        rectShapeA.setOutlineThickness(1);
        rectShapeA.setPosition(position);
        window.draw(rectShapeA);
        for (int index = 0; index < xLimit * yLimit; index++)
        {
            int x = index % xLimit;
            int y = floor(index / xLimit);
            if (structure[y][x] == 1) 
            {
                sf::RectangleShape rectShapeB;
                rectShapeB.setSize(sf::Vector2f(gridUnitSize.x, gridUnitSize.y));
                rectShapeB.setOutlineColor(sf::Color::Black);
                rectShapeB.setFillColor(colorChoices[type]);
                rectShapeB.setOutlineThickness(1);
                rectShapeB.setPosition(position.x + x*gridUnitSize.x,
                                       position.y + y*gridUnitSize.y);
                window.draw(rectShapeB);
            }
        }
    }

};

int main()
{
    //Randomer randomer{ 0,100 };
    UserInterface mainUI("arialbd.ttf");
    Map mainMap( sf::Vector2<int>(10,24), sf::Vector2<int>(0,-4), sf::Vector2<int>(32,32) ); //(mapSize in grid units, mapOffset in grid units, mapDrawOffset in pixels)
    sf::Clock clock;
    Tetranimo player;
    int score = 0;
    int highscore = 0;
    bool gameOver = false;
    sf::Time deltaTime;
    sf::RenderWindow window(sf::VideoMode(596, 704), "Tetris");
    float elapsedTime = 0;
    float completeTimer = 0; //timer for row completion animation.
    float currentFallSpeed = 2; //inverted,higher number means slower fall rate
    player.setTetranimoType(player.getNextTetranimoType());
    player.nextType = player.getNextTetranimoType();
    player.newStructure = player.listOfStructures[player.nextType];
    player.fallSpeed = currentFallSpeed;
    std::vector<int> completeRows;
    bool rowComplete = false;
    while (window.isOpen())
    {
        deltaTime = clock.restart();
        elapsedTime += deltaTime.asSeconds();
        
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::KeyReleased:
                    if (event.key.code == sf::Keyboard::Q) {
                        player.rotateStructure(PI_VALUE / 2, mainMap);
                        std::cout << "Released Q, Rotate Tetranimo" << std::endl;
                    }
                    else if (event.key.code == sf::Keyboard::E) {
                        player.rotateStructure(-(PI_VALUE / 2), mainMap);
                        std::cout << "Released E, Rotate Tetranimo" << std::endl;
                    } else if (event.key.code == sf::Keyboard::Down) {
                        //player.updatePosition(0, 1);
                        player.fallSpeed = currentFallSpeed;
                        std::cout << "Pressing Down, move Teranimo down until collision." << std::endl;
                    }
                    break;
                case sf::Event::KeyPressed:
                    if (!gameOver) {
                        if (event.key.code == sf::Keyboard::Left) {
                            player.updatePosition(-1, 0, mainMap);
                            std::cout << "Pressing Left, move Tetranimo left." << std::endl;
                        }
                        else if (event.key.code == sf::Keyboard::Right) {
                            player.updatePosition(1, 0, mainMap);
                            std::cout << "Pressing Right, move Tetranimo right." << std::endl;
                        }
                        else if (event.key.code == sf::Keyboard::Down) {
                            player.fallSpeed = 0.05;
                            std::cout << "Pressing Down, move Teranimo down until collision." << std::endl;
                        }
                    } else {
                        gameOver = false;
                    }
                    
                    break;
                case sf::Event::Closed:
                    window.close();
                    break;
            }
           
        }
        window.clear(sf::Color(116,95,127,255));
        mainUI.drawText(window, "Next:", sf::Vector2f(368, 32));
        mainUI.drawNextTetranimo(window, player.nextType, player.newStructure, sf::Vector2f(412, 76), mainMap.gridUnitSize, sf::Color(200, 175, 215, 255));
        mainUI.drawText(window, "Score:", sf::Vector2f(368, 216));
        mainUI.drawText(window, std::to_string(score), sf::Vector2f(432, 248));
        mainUI.drawText(window, "Highscore:", sf::Vector2f(368, 312));
        mainUI.drawText(window, std::to_string(highscore), sf::Vector2f(432, 344));
        if (rowComplete)
            completeTimer += deltaTime.asSeconds();
        if (!gameOver && (elapsedTime >= player.fallSpeed)) {
            if (!player.updatePosition(0, 1, mainMap)) { //update failed,hit bottom
                if (player.position.y < abs(mainMap.mapOffset.y))
                {
                    gameOver = true;
                    if (score > highscore)
                        highscore = score;
                    score = 0;
                    currentFallSpeed = 2;
                    player.reset();
                    mainMap.reset();
                }
                else {
                    mainMap.setAtPosition(player.type, player.position, player.structure);
                    completeRows = mainMap.checkForCompletedLines();
                    if (!completeRows.empty())
                    {
                        rowComplete = true;
                        score += 25*exp(completeRows.size());
                        mainMap.markForDeletion(completeRows);
                        currentFallSpeed = fmax(0.1, currentFallSpeed - 0.25 * (1 / exp(4 - completeRows.size())));
                        //add logic for calculating score and increasing fall speed here.
                    }
                    player.reset();
                }
                
            }
            elapsedTime = 0;
        }
        if ((rowComplete) && (completeTimer > 0.5))
        {
            rowComplete = false;
            completeTimer = 0;
            mainMap.deleteMarkedRows();
        }
        mainMap.drawSelf(window);
        player.drawSelf(window, mainMap);
        if (gameOver) {
            mainUI.drawText(window, "GAMEOVER", sf::Vector2f(window.getSize().x / 2 - 128, window.getSize().y / 2 + 128));
            mainUI.drawText(window, "Press any key to continue!", sf::Vector2f(window.getSize().x / 2 - 128, window.getSize().y / 2 + 192));
        }
        window.display();
    }
    return 0;
}