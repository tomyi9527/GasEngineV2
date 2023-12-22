
GASEngine.Clock = function ( autoStart ) {

	this.autoStart = ( autoStart !== undefined ) ? autoStart : true;

	this.startTime = 0;
	this.oldTime = 0;
	this.elapsedTime = 0;
    this.running = false;
    
    GASEngine.Clock.Instance = this;
};

GASEngine.Clock.prototype = {

	constructor: GASEngine.Clock,

	start: function () {
		this.startTime = performance.now();
		this.oldTime = this.startTime;
		this.running = true;

	},

	stop: function () {

		this.getElapsedTime();
		this.running = false;

	},

	getElapsedTime: function () {
		this.getDelta();
		return this.elapsedTime;
	},

	getDelta: function () {
		var diff = 0;
		if ( this.autoStart && ! this.running ) {
			this.start();
		}

		if ( this.running ) {
			var newTime = performance.now();
			diff = 0.001 * ( newTime - this.oldTime );
			this.oldTime = newTime;
			this.elapsedTime += diff;

		}
		return diff;
	}
};

GASEngine.UTF8ArrayToJSString = function(array) 
{
    var out, i, len, c;
    var char2, char3, char4;
    out = "";
    len = array.length;
    i = 0;
    while(i < len)
    {
        c = array[i++];
        switch(c >> 4)
        {
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                // 0xxxxxxx
                out += String.fromCharCode(c);
                break;
            case 12: case 13:
                // 110x xxxx   10xx xxxx
                char2 = array[i++];
                out += String.fromCharCode(((c & 0x1F) << 6) | (char2 & 0x3F));
                break;
            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                char2 = array[i++];
                char3 = array[i++];
                out += String.fromCharCode(((c & 0x0F) << 12) |
                    ((char2 & 0x3F) << 6) |
                    ((char3 & 0x3F) << 0));
                break;
            case 15:
                // 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
                char2 = array[i++];
                char3 = array[i++];
                char4 = array[i++];
                out += String.fromCodePoint(((c & 0x07) << 18) | ((char2 & 0x3F) << 12) | ((char3 & 0x3F) << 6) | (char4 & 0x3F));
                break;
        }
    }
    return out;
};

//Shapes
GASEngine.Shape = function()
{
};

GASEngine.Shape.prototype =
{
    constructor: GASEngine.Shape
};

//AABB
GASEngine.AABB = function()
{
    this.min = new GASEngine.Vector3();
    this.max = new GASEngine.Vector3();
};

GASEngine.AABB.prototype = Object.create(GASEngine.Shape);
GASEngine.AABB.prototype.constructor = GASEngine.AABB;

GASEngine.AABB.prototype.getRadius = (function()
{
    var delta = new GASEngine.Vector3();
    return function()
    {
        delta.subVectors(this.max, this.min);
        return delta.length() / 2.0;
    };
})();

GASEngine.AABB.prototype.getCenter = function(center)
{
    center.addVectors(this.min, this.max);
    center.multiplyScalar(0.5);
    return center;
};

GASEngine.AABB.prototype.fromBox = function(box)
{
    this.min.set(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);
    this.max.set(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);

    for(var i = 0; i < box.verts.length; ++i)
    {
        if(box.verts[i].x < this.min.x)
            this.min.x = box.verts[i].x;

        if(box.verts[i].y < this.min.y)
            this.min.y = box.verts[i].y;

        if(box.verts[i].z < this.min.z)
            this.min.z = box.verts[i].z;

        if(box.verts[i].x > this.max.x)
            this.max.x = box.verts[i].x;

        if(box.verts[i].y > this.max.y)
            this.max.y = box.verts[i].y;

        if(box.verts[i].z > this.max.z)
            this.max.z = box.verts[i].z;
    }
}

GASEngine.AABB.prototype.setBox = function(box)
{
    var minX = Infinity;
    var minY = Infinity;
    var minZ = Infinity;

    var maxX = Number.NEGATIVE_INFINITY;
    var maxY = Number.NEGATIVE_INFINITY;
    var maxZ = Number.NEGATIVE_INFINITY;

    for(var i = 0; i < box.verts.length; ++i)
    {
        var p = box.verts[i];
        if (minX > p.x) minX = p.x;
        if (minY > p.y) minY = p.y;
        if (minZ > p.z) minZ = p.z;

        if (maxX < p.x) maxX = p.x;
        if (maxY < p.y) maxY = p.y;
        if (maxZ < p.z) maxZ = p.z;
    }

    this.min.set(minX, minY, minZ);
    this.max.set(maxX, maxY, maxZ);
}

GASEngine.AABB.prototype.reset = function(pos)
{
    this.min.set(pos.x, pos.y, pos.z);
    this.max.set(pos.x, pos.y, pos.z);
}

GASEngine.AABB.prototype.merge = function(shape)
{
    if(shape instanceof GASEngine.Box)
    {
        for(var i = 0; i < shape.verts.length; ++i)
        {
            if(shape.verts[i].x < this.min.x)
                this.min.x = shape.verts[i].x;

            if(shape.verts[i].y < this.min.y)
                this.min.y = shape.verts[i].y;

            if(shape.verts[i].z < this.min.z)
                this.min.z = shape.verts[i].z;

            if(shape.verts[i].x > this.max.x)
                this.max.x = shape.verts[i].x;

            if(shape.verts[i].y > this.max.y)
                this.max.y = shape.verts[i].y;

            if(shape.verts[i].z > this.max.z)
                this.max.z = shape.verts[i].z;
        }
    }
    else if(shape instanceof GASEngine.AABB)
    {
        if(shape.min.x < this.min.x)
            this.min.x = shape.min.x;

        if(shape.min.y < this.min.y)
            this.min.y = shape.min.y;

        if(shape.min.z < this.min.z)
            this.min.z = shape.min.z;

        if(shape.max.x > this.max.x)
            this.max.x = shape.max.x;

        if(shape.max.y > this.max.y)
            this.max.y = shape.max.y;

        if(shape.max.z > this.max.z)
            this.max.z = shape.max.z;
    }
    else
    {
        console.error('GASEngine.AABB.merge: Can not merge the shape of the specified type.');
    }
}

//Sphere
GASEngine.Sphere = function()
{
    this.center = new GASEngine.Vector3();
    this.radius = 0.0;
};

GASEngine.Sphere.prototype = Object.create(GASEngine.Shape);
GASEngine.Sphere.prototype.constructor = GASEngine.Sphere;

GASEngine.Sphere.prototype.fromAABB = (function(aabb)
{
    var deltaVector = new GASEngine.Vector3();

    return function(aabb)
    {
        this.center.addVectors(aabb.min, aabb.max);
        this.center.multiplyScalar(0.5);

        deltaVector.subVectors(aabb.max, aabb.min);
        this.radius = deltaVector.length() / 2.0;
    };

})();

//Box
GASEngine.Box = function()
{
    this.verts = [];
    this.verts.length = 8;
};

GASEngine.Box.prototype = Object.create(GASEngine.Shape);
GASEngine.Box.prototype.constructor = GASEngine.Box;

GASEngine.Box.prototype.fromAABB = function(aabb)
{
    //y-up coordinate
    this.verts[0] = new GASEngine.Vector3(aabb.max.x, aabb.max.y, aabb.max.z);
    this.verts[1] = new GASEngine.Vector3(aabb.max.x, aabb.max.y, aabb.min.z);
    this.verts[2] = new GASEngine.Vector3(aabb.min.x, aabb.max.y, aabb.min.z);
    this.verts[3] = new GASEngine.Vector3(aabb.min.x, aabb.max.y, aabb.max.z);

    this.verts[4] = new GASEngine.Vector3(aabb.min.x, aabb.min.y, aabb.min.z);
    this.verts[5] = new GASEngine.Vector3(aabb.min.x, aabb.min.y, aabb.max.z);
    this.verts[6] = new GASEngine.Vector3(aabb.max.x, aabb.min.y, aabb.max.z);
    this.verts[7] = new GASEngine.Vector3(aabb.max.x, aabb.min.y, aabb.min.z);
};

GASEngine.Box.prototype.transform = function(matrix)
{
    for(var i = 0; i < this.verts.length; ++i)
    {
        this.verts[i].applyMatrix4(matrix);
    }
};

//Frustum
GASEngine.Frustum = function()
{

};

GASEngine.Frustum.prototype = Object.create(GASEngine.Shape);
GASEngine.Frustum.prototype.constructor = GASEngine.Frustum;


//Ray
GASEngine.Ray = function(origin, direction)
{
    this.origin = (origin !== undefined) ? origin : new GASEngine.Vector3();
    this.direction = (direction !== undefined) ? direction : new GASEngine.Vector3();
}

GASEngine.Ray.prototype = Object.create(GASEngine.Shape);
GASEngine.Ray.prototype.constructor = GASEngine.Ray;

GASEngine.Ray.prototype.set = function(origin, direction) 
{
    this.origin.copy(origin);
    this.direction.copy(direction);
}

GASEngine.Ray.prototype.clone = function()
{
    return new this.constructor().copy(this);
}

GASEngine.Ray.prototype.copy = function(ray)
{
    this.origin.copy(ray.origin);
    this.direction.copy(ray.direction);
    return this;
}

GASEngine.Ray.prototype.at = function(t, target) 
{
    if(target === undefined)
    {
        console.warn('NeotateFoundation.Ray: .at() target is now required');
        target = new GASEngine.Vector3();
    }
    target.copy(this.direction);
    target.multiplyScalar(t);
    target.add(this.origin);
    return target;
}

GASEngine.Ray.prototype.lookAt = function(v) 
{
    this.direction.copy(v);
    this.direction.sub(this.origin);
    this.direction.nromalize();
    return this;
}

GASEngine.Ray.prototype.recast = function() 
{
    var v1 = new GASEngine.Vector3();
    return function recast(t) {
        this.origin.copy(this.at(t, v1));
        return this;
    }
}

GASEngine.Ray.prototype.closestPointToPoint = function(point, target)
{
    if(target === undefined)
    {
        console.warn('NeotateFoundation.Ray: .closestPointToPoint() target is now required');
        target = new GASEngine.Vector3();
    }
    target.subVectors(point, this.origin);
    var directionDistance = target.dot(this.direction);
    if(directionDistance < 0) 
    {
        return target.copy(this.origin);
    }
    target.copy(this.direction);
    target.multiplyScalar(directionDistance);
    target.add(this.origin);
    return target;
}

GASEngine.Ray.prototype.distanceToPoint = function(point)
{
    return Math.sqrt(this.distanceSqToPoint(point));
}

GASEngine.Ray.prototype.distanceSqToPoint = function(point)
{
    var v1 = new GASEngine.Vector3();
    v1.subVectors(point, this.origin);
    var directionDistance = v1.dot(this.direction);
    if(directionDistance < 0)
    {
        return this.origin.distanceToSquared(point);
    }
    v1.copy(this.direction);
    v1.multiplyScalar(directionDistance);
    v1.add(this.origin);
    return v1.distanceToSquared(point);
}

GASEngine.Ray.prototype.distanceSqToSegment = function(v0, v1, optionalPointOnRay, optionalPointOnSegment)
{
    var segCenter = new GASEngine.Vector3();
    var segDir = new GASEngine.Vector3();
    var diff = new GASEngine.Vector3();
    
    // from http://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistRaySegment.h
    // It returns the min distance between the ray and the segment
    // defined by v0 and v1
    // It can also set two optional targets :
    // - The closest point on the ray
    // - The closest point on the segment
    segCenter.copy(v0).add(v1).multiplyScalar(0.5);
    segDir.copy(v1).sub(v0).normalize();
    diff.copy(this.origin).sub(segCenter);

    var segExtent = v0.distanceTo(v1) * 0.5;
    var a01 = -this.direction.dot(segDir);
    var b0 = diff.dot(this.direction);
    var b1 = -diff.dot(segDir);
    var c = diff.lengthSq();
    var det = Math.abs(1 - a01 * a01);
    var s0, s1, sqrDist, extDet;
    if(det > 0) 
    {
        // The ray and segment are not parallel.
        s0 = a01 * b1 - b0;
        s1 = a01 * b0 - b1;
        extDet = segExtent * det;
        if(s0 >= 0)
        {
            if(s1 >= -extDet)
            {
                if(s1 <= extDet)
                {
                    // region 0
                    // Minimum at interior points of ray and segment.
                    var invDet = 1 / det;
                    s0 *= invDet;
                    s1 *= invDet;
                    sqrDist = s0 * (s0 + a01 * s1 + 2* b0) + s1 * (a01 * s0 + s1 + 2* b1) +c;
                }
                else 
                {
                    //region 1
                    s1 = segExtent;
                    s0 = Math.max(0, -(a01 * s1 + b0));
                    sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
                }
            }
            else 
            {
                //region 5
                s1 = -segExtent;
                s0 = Math.max(0, -(a01 * s1 + b0));
                sqrDist = - s0 * s0 + s1 * (s1 + 2 * b1) + c;
            }
        }
        else 
        {
            if(s1 <= -extDet)
            {
                //region 4
                s0 = Math.max(0, -(-a01 * segExtent + b0));
                s1 = (s0 > 0) ? -segExtent : Math.min(Math.max(-segExtent, -b1), segExtent);
                sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
            }
            else if(s1 <= extDet)
            {
                //region 3
                s0 = 0;
                s1 = Math.min(Math.max(-segExtent, -b1), segExtent);
                sqrtDist = s1 * (s1 + 2 * b1) + c;
            }
            else 
            {
                //region 2
                s0 = Math.max(0, -(a01 * segExtent + b0));
                s1 = (s0 > 0) ? segExtent : Math.min(Math.max(-segExtent, -b1), segExtent);
                sqrDist = -s0 * s0 + s1 *(s1 + 2* b1) + c;
            }
        }
    }
    else 
    {
        // Ray and segment are parallel.
        s1 = (a01 > 0) ? -segExtent : segExtent;
        s0 = Math.max(0, -(a01 * s1 + b0));
        sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
    }

    if( optionalPointOnRay)
    {
        optionalPointOnRay.copy(this.direction);
        optionalPointOnRay.multiplyScalar(s0);
        optionalPointOnRay.add(this.origin);
    }
    if(optionalPointOnSegment)
    {
        optionalPointOnSegment.copy(segDir);
        optionalPointOnSegment.multiplyScalar(s1);
        optionalPointOnSegment.add(segCenter);
    }
    return sqrDist;
}

GASEngine.Ray.prototype.intersectSphere = function(sphere, target) 
{
    var v1 = new GASEngine.Vector3();
    v1.subVectors(sphere.center, this.origin);
    var tca = v1.dot(this.direction);
    var d2 = v1.dot(v1) - tca * tca;
    var radius2 = sphere.radius * sphere.radius;

    if (d2 > radius2) return null;

    var thc = Math.sqrt(radius2 - d2);

    // t0 = first intersect point - entrance on front of sphere
    var t0 = tca - thc;

    // t1 = second intersect point - exit point on back of sphere
    var t1 = tca + thc;

    // test to see if both t0 and t1 are behind the ray - if so, return null
    if (t0 < 0 && t1 < 0) return null;

    // test to see if t0 is behind the ray:
    // if it is, the ray is inside the sphere, so return the second exit point scaled by t1,
    // in order to always return an intersect point that is in front of the ray.
    if (t0 < 0) return this.at(t1, target);

    // else t0 is in front of the ray, so return the first collision point scaled by t0
    return this.at(t0, target);
}

GASEngine.Ray.prototype.intersectsSphere = function(sphere)
{
    return this.distanceSqToPoint(sphere.center) <= (sphere.radius * sphere.radius);
}

GASEngine.Ray.prototype.distanceToPlane = function(plane)
{
    var denominator = plane.normal.dot(this.direction);
    if(denominator === 0)
    {
        // line is coplanar, return origin
        if(plane.distanceToPoint(this.origin) === 0)
        {
            return 0;
        }

        // Null is preferable to undefined since undefined means.... it is undefined
        return null;
    }
    var t = -(this.origin.dot(plane.normal) + plane.constant) / denominator;

    return t >= 0 ? t : null;
}

GASEngine.Ray.prototype.intersectPlane = function(plane, target)
{
    var t = this.distanceToPlane(plane);
    if(t === null)
    {
        return null;
    }
    return this.at(t, target);
}

GASEngine.Ray.prototype.intersectsPlane = function(plane)
{
    // check if the ray lies on the plane first
    // var distToPoint = plane.distanceToPoint(this.origin);
    var distToPoint = plane.normal.dot(this.origin) + plane.constant;
    if (distToPoint === 0) 
    {
        return true;
    }

    var denominator = plane.normal.dot(this.direction);
    if (denominator * distToPoint < 0) 
    {
        return true;
    }
    // ray origin is behind the plane (and is pointing behind it)
    return false;
}

GASEngine.Ray.prototype.intersectBox = function(box, target)
{
    var tmin, tmax, tymin, tymax, tzmin, tzmax;
    var invdirx = 1 / this.direction.x,
        invdiry = 1 / this.direction.y,
        invdirz = 1 / this.direction.z;
    var origin = this.origin;
    if(invdirx >= 0)
    {
        tmin = (box.min.x - origin.x) * invdirx;
        tmax = (box.max.x - origin.x) * invdirx;
    }
    else 
    {
        tmin = (box.max.x - origin.x) * invdirx;
        tmax = (box.min.x - origin.x) * invdirx;
    }
    if (invdiry >= 0) 
    {
        tymin = (box.min.y - origin.y) * invdiry;
        tymax = (box.max.y - origin.y) * invdiry;
    } 
    else 
    {
        tymin = (box.max.y - origin.y) * invdiry;
        tymax = (box.min.y - origin.y) * invdiry;
    }

    if ((tmin > tymax) || (tymin > tmax)) return null;

    // These lines also handle the case where tmin or tmax is NaN
    // (result of 0 * Infinity). x !== x returns true if x is NaN

    if (tymin > tmin || tmin !== tmin) tmin = tymin;

    if (tymax < tmax || tmax !== tmax) tmax = tymax;

    if (invdirz >= 0) 
    {
        tzmin = (box.min.z - origin.z) * invdirz;
        tzmax = (box.max.z - origin.z) * invdirz;

    } 
    else 
    {
        tzmin = (box.max.z - origin.z) * invdirz;
        tzmax = (box.min.z - origin.z) * invdirz;
    }

    if ((tmin > tzmax) || (tzmin > tmax)) return null;

    if (tzmin > tmin || tmin !== tmin) tmin = tzmin;

    if (tzmax < tmax || tmax !== tmax) tmax = tzmax;

    //return point closest to the ray (positive side)

    if (tmax < 0) return null;
    return this.at(tmin >= 0 ? tmin : tmax, target);
}

GASEngine.Ray.prototype.intersectsBox = function(box)
{
    var v = new GASEngine.Vector3();
    return this.intersectBox(box, v) !== null;
}

GASEngine.Ray.prototype.intersectTriangle = function(a, b, c, backfaceCulling, target) 
{
    // Compute the offset origin, edges, and normal.
    var diff = new GASEngine.Vector3();
    var edge1 = new GASEngine.Vector3();
    var edge2 = new GASEngine.Vector3();
    var normal = new GASEngine.Vector3();

    // from http://www.geometrictools.com/GTEngine/Include/Mathematics/GteIntrRay3Triangle3.h

    edge1.subVectors(b, a);
    edge2.subVectors(c, a);
    normal.crossVectors(edge1, edge2);

    // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
    // E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
    //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
    //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
    //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
    var DdN = this.direction.dot(normal);
    var sign;

    if (DdN > 0) 
    {
        if (backfaceCulling) return null;
        sign = 1;
    } 
    else if (DdN < 0) 
    {
        sign = - 1;
        DdN = - DdN;
    } 
    else 
    {
        return null;
    }
    diff.subVectors(this.origin, a);
    var DdQxE2 = sign * this.direction.dot(edge2.crossVectors(diff, edge2));

    // b1 < 0, no intersection
    if (DdQxE2 < 0) 
    {
        return null;
    }

    var DdE1xQ = sign * this.direction.dot(edge1.cross(diff));

    // b2 < 0, no intersection
    if (DdE1xQ < 0) 
    {
        return null;
    }

    // b1+b2 > 1, no intersection
    if (DdQxE2 + DdE1xQ > DdN) 
    {
        return null;
    }

    // Line intersects triangle, check if ray does.
    var QdN = - sign * diff.dot(normal);

    // t < 0, no intersection
    if (QdN < 0) 
    {
        return null;
    }

    // Ray intersects triangle.
    return this.at(QdN / DdN, target);
}

GASEngine.Ray.prototype.applyMatrix4 = function(matrix4)
{
    this.origin.applyMatrix4(matrix4);
    this.direction.transformDirection(matrix4);
    return this;
}

GASEngine.Ray.prototype.equals = function(ray)
{
    return ray.origin.equals(this.origin) && ray.direction.equals(this.direction);
}

//Raycaster
GASEngine.Raycaster = function(origin, direction, near, far) 
{
    this.ray = new GASEngine.Ray(origin, direction);
    this.near = near || 0;
    this.far = far || Infinity;
    this.linePrecision = 1;
}

GASEngine.Raycaster.prototype = 
{
    constructor: GASEngine.Raycaster,
    setFromCamera: function(mouseCoordinates, cameraComponent) 
    {
        if((cameraComponent && cameraComponent.type === 'perspective')) 
        {
            this.ray.origin.setFromMatrixPosition(cameraComponent.getWorldMatrix());
            var matrix4 = new GASEngine.Matrix4();
            matrix4.getInverse(cameraComponent.getProjectionMatrix());
            this.ray.direction.set(mouseCoordinates.x, mouseCoordinates.y, 0.5);
            this.ray.direction.applyMatrix4(matrix4);
            this.ray.direction.applyMatrix4(cameraComponent.getWorldMatrix());
            this.ray.direction.sub(this.ray.origin);
            this.ray.direction.normalize();
        } 
        else if((cameraComponent && cameraComponent.type === 'orthographic')) 
        {
            // set origin in plane of camera
            this.ray.origin.set(mouseCoordinates.x, mouseCoordinates.y, (cameraComponent.near + cameraComponent.far) / (cameraComponent.near - cameraComponent.far));
            this.ray.origin.unproject(cameraComponent); 
            this.ray.direction.set(0, 0, -1);
            this.ray.direction.transformDirection(cameraComponent.getWorldMatrix());
        } 
        else 
        {
			console.error('GASEngine.Raycaster: Unsupported camera type.');
		}
    },

    intersectObject: function(entity, optionalTarget) 
    {
        var intersects = optionalTarget || [];
        this.intersectObject_r(entity, intersects);
		intersects.sort((a,b)=>{return a.distance - b.distance;});
		return intersects;
    },
    
    intersectObjects: function(entities, optionalTarget) 
    {
		var intersects = optionalTarget || [];
        if(Array.isArray(entities) === false) 
        {
			console.warn('GASEngine.Raycaster.intersectObjects: entities is not an Array.');
			return intersects;
		}

        for(var i = 0, l = entities.length; i < l; i++) 
        {
			this.intersectObject_r(entities[i], intersects);
		}
        intersects.sort((a,b)=>{return a.distance - b.distance;});
		return intersects;
    },
    intersectObject_r: function(entity, intersects) 
    {
        // var inverseMatrix = new GASEngine.Matrix4();
        // inverseMatrix.getInverse(entity.matrixWorld);
        // var ray = new GASEngine.Ray();
        /**
         Tips:
        helper的bbox为world坐标，ray也要为world坐标；
        object的bbox为local坐标，ray需要转换为local坐标
        **/
        // ray.copy(this.ray).applyMatrix4(inverseMatrix);
        // // var tmpRay = entity.type === 'helper' || entity.type === 'box' ? this.ray : ray;
        // var worldFlag = entity.isBboxWorld();
        // var tmpRay = worldFlag ? this.ray : ray;

        var worldFlag = entity.isPositionWorld();

        tmpRay = this.ray;
        if(entity.bbox !== null) 
        {
            //check the bounding by aabb method
            var boundingCheckFlag = tmpRay.intersectsBox(entity.bbox);
            if(boundingCheckFlag === false) 
            {
                return;
            }
        }
        
        var meshFilterComponent = entity.getComponent('meshFilter');
        if(meshFilterComponent !== null) 
        {
            var mesh = meshFilterComponent.getMesh();
            var index = mesh.getStream('index');
            var position, intersection = {};

            var morphed = false, skinned = false;
            if (mesh.isMorphed())
            {
                GASEngine.Utilities.cpuMorph(mesh, entity.matrixWorld);
                morphed = true;
            }

            if(mesh.isSkinned()) 
            {
                GASEngine.Utilities.cpuSkinning(mesh);
                skinned = true;
            }

            if(morphed || skinned)
            {
                position = mesh.getStream('skinnedPosition');
            }
            else 
            {
                position = mesh.getStream('position');
                worldFlag = false;
            }
           
            if(mesh.drawMode === 'TRIANGLES') 
            {  
                for(var i = 0, l = index.length; i < l; i += 3) 
                {
                    var pA = new GASEngine.Vector3();
                    var pB = new GASEngine.Vector3();
                    var pC = new GASEngine.Vector3();
                    pA.fromArray(position, index[i] * 3);
                    pB.fromArray(position, index[i + 1] * 3);
                    pC.fromArray(position, index[i + 2] * 3);

                    if (!worldFlag)
                    {
                        pA.applyMatrix4(entity.matrixWorld);
                        pB.applyMatrix4(entity.matrixWorld);
                        pC.applyMatrix4(entity.matrixWorld);
                    }

                    intersection = this.intersectTriangle(pA, pB, pC, tmpRay, entity);
                    if(intersection) 
                    {
                        intersection.face = {a: index[i], b: index[i + 1], c: index[i + 2]};
                        intersection.faceIndex = index[i] * 3;
                        intersection.object = entity;
                        intersects.push(intersection);
                    }
                }
            }
            else 
            {
                //TODO: drawMode maybe other format
            } 
        }

        //find the intersectObject recursivly
        var children = entity.children;
        for(var i = 0, l = children.length; i < l; i ++) 
        {
            this.intersectObject_r(children[i], intersects);
        }
    },

    intersectTriangle: function(pA, pB, pC, ray1, entity) 
    {   
        var intersectPoint = new GASEngine.Vector3();
        var intersect, distance, ray;
         /**
         Tips:
        helper的position为local坐标，ray也要为local坐标；
        object的position为world坐标，ray需要转换为world坐标
        **/
        // ray = entity.type === 'helper' || entity.type === 'box' ? ray1 : this.ray;
        // var worldFlag = entity.isPositionWorld();
        // ray = worldFlag ? this.ray : ray1;
        ray = this.ray;
        intersect = ray.intersectTriangle(pA, pB, pC, false, intersectPoint);
        if(intersect === null) 
        {
            return null;
        }

        // if(entity.type === 'helper'|| entity.type === 'box')
        // if(!worldFlag)
        // {
        //     var intersectionPointWorld = new GASEngine.Vector3();
        //     intersectionPointWorld.copy( intersectPoint );
        //     intersectionPointWorld.applyMatrix4( entity.matrixWorld );
        //     intersectPoint.copy(intersectionPointWorld);
        // }
        // world to world
        distance = this.ray.origin.distanceTo(intersectPoint);
        if(distance < this.near || distance > this.far) 
        {
            return null;
        }

        return {
            distance: distance,
            point: intersectPoint.clone()
        }
    }
}