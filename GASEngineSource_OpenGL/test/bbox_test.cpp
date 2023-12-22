#include "utils/bbox.h"
#include <assert.h>

void f1();
void f2();

int main() {
    f1();
    f2();
    return 0;
}

void f1() {
    BBOX<3> a;
    a.Reset(glm::vec3(0, 0, 0));
    BBOX<3> b;
    b.Reset(glm::vec3(1, 1, -1));
    a.Merge(b);
    assert(glm::all(glm::equal(a.GetCenter(), glm::vec3(0.5, 0.5, -0.5))));
    assert(glm::all(glm::equal(a.GetRadius(), glm::vec3(0.5, 0.5, 0.5))));
}
void f2() {
    BBOX<2> a;
    a.Reset(glm::vec2(0, 0));
    BBOX<2> b;
    b.Reset(glm::vec2(1, -1));
    a.Merge(b);
    assert(glm::all(glm::equal(a.GetCenter(), glm::vec2(0.5, -0.5))));
    assert(glm::all(glm::equal(a.GetRadius(), glm::vec2(0.5, 0.5))));
}
