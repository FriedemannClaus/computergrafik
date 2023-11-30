#include "math.h"
#include "geometry.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

// Die folgenden Kommentare beschreiben Datenstrukturen und Funktionen
// Die Datenstrukturen und Funktionen die weiter hinten im Text beschrieben sind,
// hängen höchstens von den vorhergehenden Datenstrukturen ab, aber nicht umgekehrt.



// Ein "Bildschirm", der das Setzen eines Pixels kapselt
// Der Bildschirm hat eine Auflösung (Breite x Höhe)
// Kann zur Ausgabe einer PPM-Datei verwendet werden oder
// mit SDL2 implementiert werden.



// Eine "Kamera", die von einem Augenpunkt aus in eine Richtung senkrecht auf ein Rechteck (das Bild) zeigt.
// Für das Rechteck muss die Auflösung oder alternativ die Pixelbreite und -höhe bekannt sein.
// Für ein Pixel mit Bildkoordinate kann ein Sehstrahl erzeugt werden.



// Für die "Farbe" benötigt man nicht unbedingt eine eigene Datenstruktur.
// Sie kann als Vector3df implementiert werden mit Farbanteil von 0 bis 1.
// Vor Setzen eines Pixels auf eine bestimmte Farbe (z.B. 8-Bit-Farbtiefe),
// kann der Farbanteil mit 255 multipliziert  und der Nachkommaanteil verworfen werden.


// Das "Material" der Objektoberfläche mit ambienten, diffusem und reflektiven Farbanteil.



// Ein "Objekt", z.B. eine Kugel oder ein Dreieck, und dem zugehörigen Material der Oberfläche.
// Im Prinzip ein Wrapper-Objekt, das mindestens Material und geometrisches Objekt zusammenfasst.
// Kugel und Dreieck finden Sie in geometry.h/tcc


// verschiedene Materialdefinition, z.B. Mattes Schwarz, Mattes Rot, Reflektierendes Weiss, ...
// im wesentlichen Variablen, die mit Konstruktoraufrufen initialisiert werden.


// Die folgenden Werte zur konkreten Objekten, Lichtquellen und Funktionen, wie Lambertian-Shading
// oder die Suche nach einem Sehstrahl für das dem Augenpunkt am nächsten liegenden Objekte,
// können auch zusammen in eine Datenstruktur für die gesamte zu
// rendernde "Szene" zusammengefasst werden.

// Die Cornelbox aufgebaut aus den Objekten
// Am besten verwendet man hier einen std::vector< ... > von Objekten.

// Punktförmige "Lichtquellen" können einfach als Vector3df implementiert werden mit weisser Farbe,
// bei farbigen Lichtquellen müssen die entsprechenden Daten in Objekt zusammengefaßt werden
// Bei mehreren Lichtquellen können diese in einen std::vector gespeichert werden.

// Sie benötigen eine Implementierung von Lambertian-Shading, z.B. als Funktion
// Benötigte Werte können als Parameter übergeben werden, oder wenn diese Funktion eine Objektmethode eines
// Szene-Objekts ist, dann kann auf die Werte teilweise direkt zugegriffen werden.
// Bei mehreren Lichtquellen muss der resultierende diffuse Farbanteil durch die Anzahl Lichtquellen geteilt werden.

// Für einen Sehstrahl aus allen Objekte, dasjenige finden, das dem Augenpunkt am nächsten liegt.
// Am besten einen Zeiger auf das Objekt zurückgeben. Wenn dieser nullptr ist, dann gibt es kein sichtbares Objekt.

// Die rekursive raytracing-Methode. Am besten ab einer bestimmten Rekursionstiefe (z.B. als Parameter übergeben) abbrechen.


const float KANTENLAENGE = 10.0;
const float TIEFE_MITTELPUNKT = 20.0;
const float RADIUS_RIESENKUGELN = 1000.0;

class Material{
public:
    Vector3df color;
    float shininess;
    float reflectivity;
    float refraction_index;

    Material(Vector3df color, float shininess, float reflectivity, float refraction_index) : color(color), shininess(shininess), reflectivity(reflectivity), refraction_index(refraction_index) {}
};

class WorldObject {
public:
    Sphere3df sphere;
    Material material;
    WorldObject(Sphere3df sphere, Material material) : sphere(sphere), material(material) {}
    bool intersects(Ray<float,3u> ray){
        return sphere.intersects(ray);
    };
};

class Scene {
public:
    Vector3df light;
    std::vector<WorldObject> objects;

public:
    Scene(Vector3df light): light(light), objects() {}

    void add_object(WorldObject object){
        objects.push_back(object);
    }
};


class Camera{
public:
    Vector3df eye;
    Vector3df direction;
    Vector3df up;
    float width;
    float height;
    float distance;
    float aspect_ratio;

    Camera(Vector<float,3> eye, Vector<float,3> direction, Vector<float,3> up, float width, float height, float distance, float aspect_ratio) {
        this->eye = eye;
        this->direction = direction;
        this->up = up;
        this->width = width;
        this->height = height;
        this->distance = distance;
        this->aspect_ratio = aspect_ratio;
    }

    Ray<float,3u> generate_ray(int x, int y){
        Vector3df ray_direction = direction + (x - width/2) * (width/2) * aspect_ratio * up + (y - height/2) * (height/2) * up;

        return Ray<float,3u>(eye, ray_direction);
    }

};

class Screen {
public:
    std::vector<Vector3df> pixels;

    int width;
    int height;
  Screen(int width, int height): pixels(){
    width = width;
    height = height;
  }
  void set_pixel(int x, int y, Vector3df color) {
    pixels[y * width + x] = color;
  }
  void write_ppm(std::ostream &out) {
    out << "P3\n" << width << " " << height << "\n255\n";
    for (auto pixel : pixels) {
      out << static_cast<int>(pixel[0] * 255) << " "
          << static_cast<int>(pixel[1] * 255) << " "
          << static_cast<int>(pixel[2] * 255) << "\n";
    }
    //write into RenderedImage.ppm
    std::ofstream file("RenderedImage.ppm");
  }
};

Vector3df raytrace(Ray<float, 3> ray, Scene scene) {
    //find the closest object
    WorldObject closestObject = scene.objects[0];
    //for all objects
    for (int i = 0; i < scene.objects.size(); i++){
        //if the ray intersects with the object
        if (scene.objects[i].intersects(ray)){
            //return the object
            closestObject = scene.objects[i];
        }
    }
    //if there is no object, return background color
    //else
    //return the color of the object
    return closestObject.material.color; //.color
}

//main() must be the last function in the file. Otherwise C++ won't get it.
int main() {
    // Bildschirm erstellen
    // Kamera erstellen
    // Für jede Pixelkoordinate x,y
    //   Sehstrahl für x,y mit Kamera erzeugen
    //   Farbe mit raytracing-Methode bestimmen
    //   Beim Bildschirm die Farbe für Pixel x,y, setzten

    //Initialize World
    Vector3df light = {0.0,KANTENLAENGE/2, TIEFE_MITTELPUNKT};
    Scene scene = Scene(light);

    Vector3df centerLeft = {-1005.0, 0.0, -20.0};
    Vector3df centerRight = {1005.0, 0.0, -20.0};
    Vector3df centerTop = {0, 1005.0, -20.0};
    Vector3df centerBottom = {0, -1005.0, -20.0};

    Sphere3df sphereLeft = Sphere3df(centerLeft, RADIUS_RIESENKUGELN);
    Sphere3df sphereRight = Sphere3df(centerRight, RADIUS_RIESENKUGELN);
    Sphere3df sphereTop = Sphere3df(centerTop, RADIUS_RIESENKUGELN);
    Sphere3df sphereBottom = Sphere3df(centerBottom, RADIUS_RIESENKUGELN);

    WorldObject objectLeft = WorldObject(sphereLeft, Material({1.0, 0.0, 0.0}, 0.0, 0.3, 0.0));
    WorldObject objectRight = WorldObject(sphereRight, Material({0.0, 1.0, 0.0}, 0.0, 0.3, 0.0));
    WorldObject objectTop = WorldObject(sphereTop, Material({0.0, 0.0, 0.0}, 0.0, 0.3, 0.0));
    WorldObject objectBottom = WorldObject(sphereBottom, Material({0.0, 0.0, 0.0}, 0.0, 0.3, 0.0));

    scene.add_object(objectLeft);
    scene.add_object(objectRight);
    scene.add_object(objectTop);
    scene.add_object(objectBottom);


    Screen screen = Screen(500, 500);

    Vector3df eye = {0.0, 0.0, 0.0};
    Vector3df direction = {0.0, 0.0, 1.0};
    Vector3df up = {0.0, 1.0, 0.0};
    Camera camera = Camera(eye, direction, up, 1, 1, 15, 1);
    for (int x = 0; x < screen.width; x++) {
        for (int y = 0; y < screen.height; y++) {
            Ray<float, 3> ray = camera.generate_ray(x, y);
            Vector3df color = raytrace(ray, scene);
            screen.set_pixel(x, y, color);
        }
    }
    screen.write_ppm(std::cout);
    return 0;
}