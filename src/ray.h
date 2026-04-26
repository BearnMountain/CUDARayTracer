#ifndef RAY_H_
#define RAY_H_

#include "vec3.h"

class ray {
public:
	ray() {}
	ray(const vec3& origin, const vec3& direction) : origin(origin), direction(direction) {}

	const vec3& getOrigin() const { return origin; }
	const vec3& getDirection() const { return direction; }

	vec3 at(double t) const { return t * direction + origin; }

private:
	vec3 origin;
	vec3 direction;
};

#endif
