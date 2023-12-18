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
class Screen {
public:
    int width;
    int height;
    std::vector<Vector3df> pixels;
    Screen(int w, int h) : width(w), height(h), pixels(w * h, Vector3df{0.0, 0.0, 0.0}) {}

    void set_pixel(int x, int y, Vector3df color) {
        //std::cout << "setting pixel " << x << y << "with color" << color[0] <<", " << color[1] << ", " << color[2] << "\n";
        pixels[y * width + x] = color;
        //std::cout << "finished setting the pixel, back to main\n";
    }
    void write_ppm() {
        //open a file
        std::ofstream file("RenderedImage.ppm");

        file << "P3\n" << width << " " << height << "\n255\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const Vector3df& color = pixels[y * width + x];
                file << static_cast<int>(color[0] * 255) << " "
                         << static_cast<int>(color[1] * 255) << " "
                         << static_cast<int>(color[2] * 255) << " ";
            }
            file << "\n";
        }
        file.close();
    }
};


// Eine "Kamera", die von einem Augenpunkt aus in eine Richtung senkrecht auf ein Rechteck (das Bild) zeigt.
// Für das Rechteck muss die Auflösung oder alternativ die Pixelbreite und -höhe bekannt sein.
// Für ein Pixel mit Bildkoordinate kann ein Sehstrahl erzeugt werden.

class Camera{
public:
    Screen screen;
    Vector3df eye;
    Vector3df direction;
    Vector3df up;
    Vector3df w;
    Vector3df u;
    Vector3df v;
    float width;
    float height;
    float distance;
    float aspect_ratio;
    float l;
    float r;
    float t;
    float b;


    Camera(Screen screen, Vector3df eye, Vector3df direction, Vector3df up, float width, float height, float distance, float aspect_ratio, Vector3df u, Vector3df v, Vector3df w): screen(screen), eye(eye), direction(direction), up(up), u(u), v(v), w(w){
        this->width = width;
        this->height = height;
        this->distance = distance;
        this->aspect_ratio = aspect_ratio;
        this->l = -width/2;
        this->r = width/2;
        this->t = height/2;
        this->b = -height/2;
    }

    Ray<float,3u> generate_ray(int x, int y){
        float p_u = l + (r-l) * (x + 0.5)/screen.width;
        float p_v = b + (t-b) * (y + 0.5)/screen.height;

        Vector3df minusOne = {-1.0, -1.0, -1.0};

        //Vector3df ray_direction = direction + (x - width/2) * (width/2) * aspect_ratio * up + (y - height/2) * (height/2) * up;
        Vector3df ray_direction = minusOne * direction * w + p_u * u + p_v * v; //-dw' + p_uu' + p_v * v'
        return Ray<float,3u>(eye, ray_direction);
    }

};

// Für die "Farbe" benötigt man nicht unbedingt eine eigene Datenstruktur.
// Sie kann als Vector3df implementiert werden mit Farbanteil von 0 bis 1.
// Vor Setzen eines Pixels auf eine bestimmte Farbe (z.B. 8-Bit-Farbtiefe),
// kann der Farbanteil mit 255 multipliziert  und der Nachkommaanteil verworfen werden.


// Das "Material" der Objektoberfläche mit ambienten, diffusem und reflektiven Farbanteil.
class Material{
public:
    Vector3df color;
    float shininess;
    float reflectivity;
    float refraction_index;

    Material(Vector3df color, float shininess, float reflectivity, float refraction_index) : color(color), shininess(shininess), reflectivity(reflectivity), refraction_index(refraction_index) {}
};


// Ein "Objekt", z.B. eine Kugel oder ein Dreieck, und dem zugehörigen Material der Oberfläche.
// Im Prinzip ein Wrapper-Objekt, das mindestens Material und geometrisches Objekt zusammenfasst.
// Kugel und Dreieck finden Sie in geometry.h/tcc
class WorldObject {
public:
    Sphere3df sphere;
    Material material;
    WorldObject(Sphere3df sphere, Material material) : sphere(sphere), material(material) {}
    float intersects(Ray<float,3u> ray, Intersection_Context<float, 3> hitContext){
        return sphere.intersects(ray, hitContext);
    };
    float intersects(Ray<float,3u> ray){
        return sphere.intersects(ray);
    };
};

// verschiedene Materialdefinition, z.B. Mattes Schwarz, Mattes Rot, Reflektierendes Weiss, ...
// im wesentlichen Variablen, die mit Konstruktoraufrufen initialisiert werden.


// Die folgenden Werte zur konkreten Objekten, Lichtquellen und Funktionen, wie Lambertian-Shading
// oder die Suche nach einem Sehstrahl für das dem Augenpunkt am nächsten liegenden Objekte,
// können auch zusammen in eine Datenstruktur für die gesamte zu
// rendernde "Szene" zusammengefasst werden.
class Scene {
public:
    Vector3df light;
    std::vector<WorldObject> objects;
    Intersection_Context<float, 3> hitContext;

public:
    Scene(Vector3df light): light(light), objects() {}

    void add_object(WorldObject object){
        objects.push_back(object);
    }


// Die Cornelbox aufgebaut aus den Objekten
// Am besten verwendet man hier einen std::vector< ... > von Objekten.

// Punktförmige "Lichtquellen" können einfach als Vector3df implementiert werden mit weisser Farbe,
// bei farbigen Lichtquellen müssen die entsprechenden Daten in Objekt zusammengefaßt werden
// Bei mehreren Lichtquellen können diese in einen std::vector gespeichert werden.

// Sie benötigen eine Implementierung von Lambertian-Shading, z.B. als Funktion
// Benötigte Werte können als Parameter übergeben werden, oder wenn diese Funktion eine Objektmethode eines
// Szene-Objekts ist, dann kann auf die Werte teilweise direkt zugegriffen werden.
// Bei mehreren Lichtquellen muss der resultierende diffuse Farbanteil durch die Anzahl Lichtquellen geteilt werden.

    Vector3df lambertian(Vector3df light, Vector3df normal, float kd) {

        // Vektoren vor der Berechnung normalisieren
        Vector3df n = normal;
        n.normalize();
        Vector3df l = light;
        l.normalize();

        // Berechne den Lambertian Shading-Term
        Vector3df brightness =  kd * (std::max(0.0f, n * l) * l); //TODO: Vielleicht muss l nicht der Vektor zum Licht vom Auge aus,
        //sondern der Vektor vom Schnittpunkt zum Licht sein.

        return brightness;
    }



// Für einen Sehstrahl aus allen Objekte, dasjenige finden, das dem Augenpunkt am nächsten liegt.
// Am besten einen Zeiger auf das Objekt zurückgeben. Wenn dieser nullptr ist, dann gibt es kein sichtbares Objekt.
    WorldObject findNearestObject(Ray3df ray){
        auto closestObject = objects[0]; //TODO: check if this is a good idea. Should always find an object
        float minimal_t = INFINITY;
        Intersection_Context<float, 3> bestHitContext;
        for (int i = 0; i < (int) objects.size(); i++){
            //if the ray intersects with the object
            //std::cout << "checking for intersection object Nr: " << i << "\n";
            float t = objects[i].intersects(ray, hitContext);
            if (t > 0 && t < minimal_t){
                //std::cout << "found an possible/nearer intersection\n";
                closestObject = objects[i];
                minimal_t = t;
                bestHitContext = hitContext; //TODO: Check that there is no pointer error for bestHitContext (if it gets overwritten)
            }
        }
        hitContext = bestHitContext;
        return closestObject;
    }

};

// Die rekursive raytracing-Methode. Am besten ab einer bestimmten Rekursionstiefe (z.B. als Parameter übergeben) abbrechen.

Vector3df raytrace(Ray<float, 3> ray, Scene scene) {
    //find the closest object
    WorldObject closestObject = scene.findNearestObject(ray);
    //if there is no object, return background color
    //else
    //return the color of the object
    return closestObject.material.color; //.color
}

const float KANTENLAENGE = 10.0;
const float TIEFE_MITTELPUNKT = 20.0;
const float RADIUS_RIESENKUGELN = 1000.0;

//main() must be the last function in the file. Otherwise C++ won't get it.
int main() {
    // Bildschirm erstellen
    // Kamera erstellen
    // Für jede Pixelkoordinate x,y
    //   Sehstrahl für x,y mit Kamera erzeugen
    //   Farbe mit raytracing-Methode bestimmen
    //   Beim Bildschirm die Farbe für Pixel x,y, setzten

    //Initialize World
    Vector3df light = {0.0, (KANTENLAENGE/2) - 0.5f, TIEFE_MITTELPUNKT};
    Scene scene = Scene(light);

    Vector3df centerLeft = {-1005.0, 0.0, -20.0};
    Vector3df centerRight = {1005.0, 0.0, -20.0};
    Vector3df centerTop = {0, 1005.0, -20.0};
    Vector3df centerBottom = {0, -1005.0, -20.0};
    Vector3df centerBack = {0, 0, -1025.0};

    Vector3df centerSphere1 = {-2.0, -2.0, -18.0};
    Vector3df centerSphere2 = {2.0, 2.0, -22.0};

    Sphere3df sphereLeft = Sphere3df(centerLeft, RADIUS_RIESENKUGELN);
    Sphere3df sphereRight = Sphere3df(centerRight, RADIUS_RIESENKUGELN);
    Sphere3df sphereTop = Sphere3df(centerTop, RADIUS_RIESENKUGELN);
    Sphere3df sphereBottom = Sphere3df(centerBottom, RADIUS_RIESENKUGELN);
    Sphere3df sphereBack = Sphere3df(centerBack, RADIUS_RIESENKUGELN);

    Sphere3df sphere1 = Sphere3df(centerSphere1, 1.5);
    Sphere3df sphere2 = Sphere3df(centerSphere2, 1.5);


    WorldObject objectLeft = WorldObject(sphereLeft, Material({1.0, 0.0, 0.0}, 0.4, 0.3, 0.0));
    WorldObject objectRight = WorldObject(sphereRight, Material({0.0, 1.0, 0.0}, 0.4, 0.3, 0.0));
    WorldObject objectTop = WorldObject(sphereTop, Material({0.9, 0.9, 0.9}, 0.4, 0.3, 0.0));
    WorldObject objectBottom = WorldObject(sphereBottom, Material({0.9, 0.9, 0.9}, 0.4, 0.3, 0.0));
    WorldObject objectBack = WorldObject(sphereBack, Material({0.9, 0.9, 0.9}, 0.4, 0.3, 0.0));

    WorldObject object1 = WorldObject(sphere1, Material({0.0, 0.0, 0.1}, 0.4, 0.3, 0.0));
    WorldObject object2 = WorldObject(sphere2, Material({0.0, 0.1, 0.1}, 0.4, 0.3, 0.0));

    scene.add_object(objectLeft);
    scene.add_object(objectRight);
    scene.add_object(objectTop);
    scene.add_object(objectBottom);
    scene.add_object(objectBack);

    scene.add_object(object1);
    scene.add_object(object2);


    Screen screen = Screen(1000, 1000);

    Vector3df eye = {0.0, 0.0, 0.0};
    Vector3df direction = {0.0, 0.0, 1.0};
    Vector3df up = {0.0, 1.0, 0.0};
    Vector3df w = Vector3df {0.0, 0.0, 1.0};
    Vector3df u = Vector3df {1.0, 0.0, 0.0};
    Vector3df v = Vector3df {0.0, 1.0, 0.0};
    Camera camera = Camera(screen, eye, direction, up, 5, 5, 15, 1, u, v, w);
    //std::cout << "before iterating\n" << "width: " << screen.width << " height: " << screen.height << "\n";
    for (int x = 0; x < screen.width; x++) {
        for (int y = 0; y < screen.height; y++) {
            //std::cout << "x: " << x << " y: " << y << " this is great, im iterating the rays\n";
            Ray<float, 3> ray = camera.generate_ray(x, y);
            auto closestObject = scene.findNearestObject(ray);
            auto baseColor = closestObject.material.color;
            Vector3df color = baseColor;//raytrace(ray, scene);
            //std::cout << "got back the color" << color[0] <<", " << color[1] << ", " << color[2] << "\n";
            color = color + scene.lambertian(light, scene.hitContext.normal, closestObject.material.shininess);
            screen.set_pixel(x, y, color);
        }
    }
    //0std::cout << "writing the file\n";
    screen.write_ppm();
    return 0;
}