#include "math.h"
#include "geometry.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

// Die folgenden Kommentare beschreiben Datenstrukturen und Funktionen
// Die Datenstrukturen und Funktionen die weiter hinten im Text beschrieben sind,
// hängen höchstens von den vorhergehenden Datenstrukturen ab, aber nicht umgekehrt.

const float KANTENLAENGE = 10.0f;
const float TIEFE_MITTELPUNKT = 20.0f;
const float RADIUS_RIESENKUGELN = 1000.0f;
const float GRUNDHELLIGKEIT = 0.26f;
const float EPSILON = 0.0001f;

// Ein "Bildschirm", der das Setzen eines Pixels kapselt
// Der Bildschirm hat eine Auflösung (Breite x Höhe)
// Kann zur Ausgabe einer PPM-Datei verwendet werden oder
// mit SDL2 implementiert werden.
class Screen {
public:
    int nx;
    int ny;
    std::vector<Vector3df> pixels;
    Screen(int nx, int ny) : nx(nx), ny(ny), pixels(nx * ny, Vector3df{0.0, 0.0, 0.0}) {}

    void set_pixel(int x, int y, Vector3df color) {
        //std::cout << "setting pixel " << x << y << "with color" << color[0] <<", " << color[1] << ", " << color[2] << "\n";
        pixels[y * nx + x] = color;
        //std::cout << "finished setting the pixel, back to main\n";
    }
    void write_ppm() {
        //open a file
        std::ofstream file("RenderedImage.ppm");

        file << "P3\n" << nx << " " << ny << "\n255\n";
        for (int y = ny - 1; y > 0; --y) {
            for (int x = 0; x < nx; ++x) {
                const Vector3df& color = pixels[y * nx + x];
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
    float l;
    float r;
    float t;
    float b;


    Camera(Screen screen, Vector3df eye, Vector3df direction, Vector3df up, float width, float height, float distance, Vector3df u, Vector3df v, Vector3df w): screen(screen), eye(eye), direction(direction), up(up), u(u), v(v), w(w){
        this->width = width;
        this->height = height;
        this->distance = distance;
        this->l = -width/2;
        this->r = width/2;
        this->t = height/2;
        this->b = -height/2;
    }

    Ray<float,3u> generate_ray(int x, int y){
        float p_u = l + (r-l) * (x + 0.5f)/screen.nx;
        float p_v = b + (t-b) * (y + 0.5f)/screen.ny;

        Vector3df minusOne = {-1.0, -1.0, -1.0};

        //Vector3df ray_direction = direction + (x - nx/2) * (nx/2) * aspect_ratio * up + (y - ny/2) * (ny/2) * up;
        //Vector3df ray_direction = minusOne * direction * w + p_u * u + p_v * v; //-dw' + p_uu' + p_v * v' //TODO: Didn't work
        Vector3df ray_direction = (Vector3df{0,0,0} -(distance *  w)) + p_u * u + p_v * v;
        ray_direction.normalize();
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
    bool intersects(Ray<float,3u> ray, Intersection_Context<float, 3> hitContext){
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

    float lambertian(Vector3df light, Vector3df normal, float kd) {

        // Vektoren vor der Berechnung normalisieren
        Vector3df n = normal;
        n.normalize();
        Vector3df l = light;
        l.normalize();

        // Berechne den Lambertian Shading-Term
        //Vector3df brightness =  kd * (std::max(0.0f, n * l) * l); //TODO: Vielleicht muss l nicht der Vektor zum Licht vom Auge aus,
        float brightness =  kd * std::max(0.0f, n * l);
        //sondern der Vektor vom Schnittpunkt zum Licht sein.

        return brightness;
    }



// Für einen Sehstrahl aus allen Objekte, dasjenige finden, das dem Augenpunkt am nächsten liegt.
// Am besten einen Zeiger auf das Objekt zurückgeben. Wenn dieser nullptr ist, dann gibt es kein sichtbares Objekt.
    WorldObject* findNearestObject(Ray3df ray){ //TODO: check if this is a good idea. Should always find an object
        WorldObject *nearestObject = nullptr;
        float minimal_t = INFINITY;
        Intersection_Context<float, 3> intersectionContext;
        for (WorldObject &o : objects){
            //if the ray intersects with the object
            bool found = o.sphere.intersects(ray, intersectionContext);
            //std::cout << "checking for intersection object Nr: " << intersectionContext.t << "\n";
            if (found && (intersectionContext.t > 0 + EPSILON) && (intersectionContext.t < minimal_t - EPSILON)){
                //std::cout << "found an possible/nearer intersection\n";
                hitContext = intersectionContext;
                nearestObject = &o;
                minimal_t = intersectionContext.t;
            }
        }
        return nearestObject;
    }

    bool findLightSourcesFromLastHitContext(){
        Ray3df shadow_ray(hitContext.intersection, light - hitContext.intersection);
        findNearestObject(shadow_ray);

        if(hitContext.t > 0 + EPSILON && hitContext.t < 1 - EPSILON){
            return false;
        } else {
            return true;
        }
    }
};

// Die rekursive raytracing-Methode. Am besten ab einer bestimmten Rekursionstiefe (z.B. als Parameter übergeben) abbrechen.

Vector3df raytrace(Ray<float, 3> ray, Scene scene, int depth) {
    Vector3df color = {GRUNDHELLIGKEIT, GRUNDHELLIGKEIT, GRUNDHELLIGKEIT};
    if(depth != 0) {
        WorldObject* closestObject = scene.findNearestObject(ray);
        if (closestObject == nullptr) {
            return color;
        }
        Material materialFound = closestObject->material;
        Vector3df normal = scene.hitContext.normal;
        Vector3df intersection = scene.hitContext.intersection;
        if(materialFound.reflectivity > 0.0f) {
            Vector3df direction = ray.direction;
            Vector3df reflection = direction - 2 * (direction * normal) * normal;
            reflection.normalize();
            Ray3df reflective = Ray3df(intersection, reflection);
            color = materialFound.color +  materialFound.reflectivity * raytrace(reflective, scene, depth - 1);
        }
        color = (scene.lambertian(scene.light - intersection, normal, materialFound.shininess) + 0.3f * GRUNDHELLIGKEIT) * color;
    }
    return color;
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
    Vector3df light = {0.0, (KANTENLAENGE/2) - 0.5f, -TIEFE_MITTELPUNKT};
    Scene scene = Scene(light);

    Vector3df centerLeft = {-1005.0, 0.0, -20.0};
    Vector3df centerRight = {1005.0, 0.0, -20.0};
    Vector3df centerTop = {0, 1005.0, -20.0};
    Vector3df centerBottom = {0, -1005.0, -20.0};
    Vector3df centerBack = {0, 0, -1025.0};

    Vector3df centerSphere1 = {-2.0, -2.0, -18.0};
    Vector3df centerSphere2 = {2.0, 2.0, -22.0};
    Vector3df centerSphere3 = {-2.5, 2.3, -23.0};

    Sphere3df sphereLeft = Sphere3df(centerLeft, RADIUS_RIESENKUGELN);
    Sphere3df sphereRight = Sphere3df(centerRight, RADIUS_RIESENKUGELN);
    Sphere3df sphereTop = Sphere3df(centerTop, RADIUS_RIESENKUGELN);
    Sphere3df sphereBottom = Sphere3df(centerBottom, RADIUS_RIESENKUGELN);
    Sphere3df sphereBack = Sphere3df(centerBack, RADIUS_RIESENKUGELN);

    Sphere3df sphere1 = Sphere3df(centerSphere1, 1.5);
    Sphere3df sphere2 = Sphere3df(centerSphere2, 1.8);
    Sphere3df sphere3 = Sphere3df(centerSphere3, 1.0);


    WorldObject objectLeft = WorldObject(sphereLeft, Material({0.9, 0.0, 0.0}, 0.4, 0.9, 0.0));
    WorldObject objectRight = WorldObject(sphereRight, Material({0.0, 0.9, 0.0}, 0.4, 0.3, 0.0));
    WorldObject objectTop = WorldObject(sphereTop, Material({0.1, 0.1, 0.1}, 0.4, 0.3, 0.0));
    WorldObject objectBottom = WorldObject(sphereBottom, Material({0.8, 0.8, 0.8}, 0.8, 0.3, 0.0));
    WorldObject objectBack = WorldObject(sphereBack, Material({0.9, 0.9, 0.9}, 0.4, 0.3, 0.0));

    WorldObject object1 = WorldObject(sphere1, Material({0.0, 0.0, 0.9}, 0.4, 0.9, 0.0));
    WorldObject object2 = WorldObject(sphere2, Material({0.0, 0.9, 0.9}, 0.4, 0.3, 0.0));
    WorldObject object3 = WorldObject(sphere3, Material({0.9, 0.9, 0.0}, 0.7, 0.94, 0.0));

    scene.add_object(objectLeft);
    scene.add_object(objectRight);
    scene.add_object(objectTop);
    scene.add_object(objectBottom);
    scene.add_object(objectBack);

    scene.add_object(object1);
    scene.add_object(object2);
    scene.add_object(object3);


    Screen screen = Screen(1000, 1000);

    Vector3df eye = {0.0, 0.0, 0.0};
    Vector3df direction = {0.0, 0.0, 1.0};
    Vector3df up = {0.0, 1.0, 0.0};
    Vector3df w = Vector3df {0.0, 0.0, 1.0};
    Vector3df u = Vector3df {1.0, 0.0, 0.0};
    Vector3df v = Vector3df {0.0, 1.0, 0.0};
    Camera camera = Camera(screen, eye, direction, up, 10.0f, 10.0f, 15.0f, u, v, w);
    //std::cout << "before iterating\n" << "nx: " << screen.nx << " ny: " << screen.ny << "\n";
    for (int x = 0; x < screen.nx; x++) {
        for (int y = 0; y < screen.ny; y++) {
            //std::cout << "x: " << x << " y: " << y << " this is great, im iterating the rays\n";
            Ray<float, 3> ray = camera.generate_ray(x, y);
            Vector3df color = raytrace(ray, scene, 3);
            WorldObject* closestObject = scene.findNearestObject(ray);
            //auto baseColor = closestObject->material.color;
            //Vector3df color = baseColor;//raytrace(ray, scene);
            auto firstIntersection = scene.hitContext.intersection;
            auto firstNormal = scene.hitContext.normal;
            if(scene.findLightSourcesFromLastHitContext()) {
                color = (scene.lambertian(light - firstIntersection, firstNormal, closestObject->material.shininess) + GRUNDHELLIGKEIT) * color;
            } else {
                color = GRUNDHELLIGKEIT * color;
            };
            screen.set_pixel(x, y, color);
        }
    }
    //0std::cout << "writing the file\n";
    screen.write_ppm();
    return 0;
}