(function()
{
    let GIZMO_PREFIX = 'Editor_Gizmo_';
    let GSIZE = 1;

    let RED_COLOR = [1.0, 0.0, 0.0, 1.0];
    let GREEN_COLOR = [0.0, 1.0, 0.0, 1.0];
    let BLUE_COLOR = [0.0, 0.0, 1.0, 1.0];

    let WHITE_COLOR = [1.0, 1.0, 1.0, 1.0];
    let WHITE_COLOR_TRANSPARENT = [1.0, 1.0, 1.0, 0.25];

    let GRAY_COLOR = [0.5, 0.5, 0.5, 1.0];

    let XY_COLOR = [1.0, 1.0, 0.0, 0.25];
    let YZ_COLOR = [0.0, 1.0, 1.0, 0.25];
    let ZX_COLOR = [1.0, 0.0, 1.0, 0.25];

    let HOVER_COLOR = [1.0, 1.0, 0.0, 1.0];
    let OTHER_COLOR = [0.7, 0.7, 0.7, 0.0];
    let SPHERE_COLOR = [0.5, 0.5, 0.5, 0.0];
    var SPHERE_HIGHLIGHT_COLOR = [0.5, 0.5, 0.5, 0.5];

    let tempVector = new GASEngine.Vector3();
    let tmpRotation = new GASEngine.Euler();
    let tempScale = new GASEngine.Vector3();

    let cameraPosition = new GASEngine.Vector3();
    let cameraQuaternion = new GASEngine.Quaternion();
    let cameraScale = new GASEngine.Vector3();

    let matrix = new GASEngine.Matrix4();
    let worldPosition = new GASEngine.Vector3(0, 1, 0);
    let unitY = new GASEngine.Vector3(0, 1, 0);
    let gizmoQuaternion = new GASEngine.Quaternion();
    let quaternionOffset = new GASEngine.Quaternion();
    quaternionOffset.setFromEuler(new GASEngine.Euler(0, Math.PI / 2, 0));
    let gizmoRotation = new GASEngine.Euler();
    let dir = new GASEngine.Vector3();
    let lookAt = new GASEngine.Vector3();

    // -------------------------------------------- gizmo util --------------------------------------------

    let ViewportGizmoUtil =
    {
    };

    ViewportGizmoUtil.createLineMesh = function(application, size)
    {
        let meshfactory = application.getMeshFactory();

        size = size || 1;
        
        let mesh = meshfactory.create();
        mesh.drawMode = 'LINES';
        mesh.type = 'line';
        let vertices = [
            0, 0, 0,	
            size, 0, 0
        ];
        let count = vertices.length / 3;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);
        mesh.submitToWebGL();

        return mesh;
    }

    ViewportGizmoUtil.createPlaneHelperMesh = function(application, width, height, widthSegments, heightSegments, halfFlag)
    {
        let meshfactory = application.getMeshFactory();

        width = width || 1;
        height = height || 1;

        let width_half = width / 2;
        let height_half = height / 2;

        let gridX = widthSegments === undefined ? 1 : widthSegments;
        let gridY = heightSegments === undefined ? 1 : heightSegments;

        let gridX1 = gridX + 1;
        let gridY1 = gridY + 1;

        let segment_width = width / gridX;
        let segment_height = height / gridY;

        let ix, iy;

        let mesh = meshfactory.create();
        mesh.type = 'plane';

        // buffers
        let indices = [];
        let vertices = [];
        let normals = [];
        let uvs = [];

        // generate vertices, normals and uvs
        for(iy = 0; iy < gridY1; iy++) 
        {
            let y = halfFlag ? (iy * segment_height - height_half) : iy *  segment_height;
            for(ix = 0; ix < gridX1; ix ++) 
            {
                if(halfFlag)
                {
                    let x = ix * segment_width - width_half;
                    vertices.push(x, - y, 0);   
                    uvs.push(ix / gridX);
                    uvs.push(1 - (iy / gridY));
                }
                else 
                {
                    let x = ix * segment_width;
                    vertices.push(x, y, 0);
                    uvs.push(ix / gridX);
                    uvs.push(iy / gridY);
                }
            
                normals.push(0, 0, 1);
            }
        }

        // indices
        for(iy = 0; iy < gridY; iy++) 
        {
            for(ix = 0; ix < gridX; ix++) 
            {
                let a = ix + gridX1 * iy;
                let b = ix + gridX1 * ( iy + 1 );
                let c = (ix + 1) + gridX1 * (iy + 1);
                let d = (ix + 1) + gridX1 * iy;

                // faces
                indices.push(a, d, b);
                indices.push(b, d, c);

                //   // faces back
                //   indices.push(a, b, d);
                //   indices.push(b, c, d);
            }
        }

        let count = indices.length;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('normal', new Float32Array(normals));
        mesh.addStream('uv', new Float32Array(uvs));
        mesh.addStream('index', new Uint32Array(indices));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);
        mesh.submitToWebGL();

        return mesh;
    }

    //arrowhelper: CylinderGeometry & ConeGeometry
    ViewportGizmoUtil.createArrowHelperMesh = function(application, radiusTop, radiusBottom, height, radialSegments, heightSegments, openEnded, thetaStart, thetaLength)
    {
        let meshfactory = application.getMeshFactory();

        radiusTop = radiusTop !== undefined ? radiusTop : 1;
        radiusBottom = radiusBottom !== undefined ? radiusBottom : 1;
        height = height || 1;

        radialSegments = Math.floor(radialSegments) || 8;
        heightSegments = Math.floor(heightSegments) || 1;

        openEnded = openEnded !== undefined ? openEnded : false;
        thetaStart = thetaStart !== undefined ? thetaStart : 0.0;
        thetaLength = thetaLength !== undefined ? thetaLength : Math.PI * 2;

        let mesh = meshfactory.create();
        mesh.type = 'arrow';

        // buffers
        let indices = [];
        let vertices = [];
        let normals = [];
        let uvs = [];

        // helper variables
        let index = 0;
        let indexArray = [];
        let halfHeight = height / 2;
        var groupStart = 0;
        // var subMesh = [];
        generateTorso();

        if (openEnded === false) 
        {

            if (radiusTop > 0) generateCap(true);
            if (radiusBottom > 0) generateCap(false);

        }
        let count = indices.length;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('normal', new Float32Array(normals));
        mesh.addStream('index', new Uint32Array(indices));
        mesh.addStream('uv', new Float32Array(uvs));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);

        mesh.submitToWebGL();

        return mesh;
        
        function generateTorso() 
        {
            let x, y;
            let normal = new GASEngine.Vector3();
            let vertex = new GASEngine.Vector3();

            let groupCount = 0;

            // this will be used to calculate the normal
            let slope = (radiusBottom - radiusTop) / height;

            // generate vertices, normals and uvs
            for (y = 0; y <= heightSegments; y++) 
            {
                let indexRow = [];
                let v = y / heightSegments;
                // calculate the radius of the current row
                let radius = v * (radiusBottom - radiusTop) + radiusTop;
                for (x = 0; x <= radialSegments; x ++) 
                {
                    let u = x / radialSegments;
                    let theta = u * thetaLength + thetaStart;
                    let sinTheta = Math.sin(theta);
                    let cosTheta = Math.cos(theta);
                    // vertex
                    vertex.x = radius * sinTheta;
                    vertex.y = - v * height + halfHeight;
                    vertex.z = radius * cosTheta;
                    vertices.push(vertex.x, vertex.y, vertex.z);
                    // normal
                    normal.set(sinTheta, slope, cosTheta).normalize();
                    normals.push(normal.x, normal.y, normal.z);
                    // uv
                    uvs.push(u, 1 - v);
                    // save index of vertex in respective row
                    indexRow.push(index++);
                }
                // now save vertices of the row in our index array
                indexArray.push(indexRow);
            }

            // generate indices
            for(x = 0; x < radialSegments; x ++) 
            {
                for(y = 0; y < heightSegments; y++) 
                {
                    // we use the index array to access the correct indices
                    let a = indexArray[y][x];
                    let b = indexArray[y + 1][x];
                    let c = indexArray[y + 1][x + 1];
                    let d = indexArray[y][x + 1];
                    // faces
                    indices.push(a, b, d);
                    indices.push(b, c, d);
                    // update group counter
                    groupCount += 6;
                }
            }

            // add a group to the geometry. this will ensure multi material support
            // scope.addGroup( groupStart, groupCount, 0 );
            // calculate new start value for groups
            groupStart += groupCount;
        }

        function generateCap(top) 
        {
            let x, centerIndexStart, centerIndexEnd;
            let uv = new GASEngine.Vector2();
            let vertex = new GASEngine.Vector3();
            let groupCount = 0;
            let radius = (top === true) ? radiusTop : radiusBottom;
            let sign = (top === true) ? 1 : - 1;
            // save the index of the first center vertex
            centerIndexStart = index;
            // first we generate the center vertex data of the cap.
            // because the geometry needs one set of uvs per face,
            // we must generate a center vertex per face/segment
            for (x = 1; x <= radialSegments; x ++) 
            {
                // vertex
                vertices.push(0, halfHeight * sign, 0);
                // normal
                normals.push(0, sign, 0);
                // uv
                uvs.push(0.5, 0.5);
                // increase index
                index ++;
            }
            // save the index of the last center vertex
            centerIndexEnd = index;
            // now we generate the surrounding vertices, normals and uvs
            for(x = 0; x <= radialSegments; x ++) 
            {
                var u = x / radialSegments;
                var theta = u * thetaLength + thetaStart;

                var cosTheta = Math.cos(theta);
                var sinTheta = Math.sin(theta);
                // vertex
                vertex.x = radius * sinTheta;
                vertex.y = halfHeight * sign;
                vertex.z = radius * cosTheta;
                vertices.push(vertex.x, vertex.y, vertex.z);

                // normal
                normals.push(0, sign, 0);

                // uv
                uv.x = (cosTheta * 0.5) + 0.5;
                uv.y = (sinTheta * 0.5 * sign) + 0.5;
                uvs.push(uv.x, uv.y);
                // increase index
                index ++;
            }

            // generate indices
            for (x = 0; x < radialSegments; x ++) 
            {
                let c = centerIndexStart + x;
                let i = centerIndexEnd + x;
                if (top === true) 
                {
                    // face top
                    indices.push(i, i + 1, c);
                } 
                else 
                {
                    // face bottom
                    indices.push(i + 1, i, c);
                }
                groupCount += 3;
            }
            groupStart += groupCount;
        }
    }

    ViewportGizmoUtil.createCircleHelperMesh = function(application, radius, segments, halfFlag, thetaStart, thetaLength)
    {
        let meshfactory = application.getMeshFactory();

        radius = radius || 1;
        segments = segments !== undefined ? Math.max( 3, segments ) : 8;

        let arc = halfFlag ? 0.5 : 1;

        thetaStart = thetaStart !== undefined ? thetaStart : 0;
        thetaLength = thetaLength !== undefined ? thetaLength : Math.PI * 2;

        let mesh = meshfactory.create();
        mesh.drawMode = 'LINES';
        mesh.type = 'circle';

        // buffers
        let indices = [];
        let vertices = [];
        let normals = [];
        let uvs = [];

        // helper variables
        let i, s;
        let vertex = new GASEngine.Vector3();
        let uv = new GASEngine.Vector2();

        for ( s = 0, i = 0; s <= segments * arc; s ++, i += 3 ) {

            let segment = thetaStart + s / segments * thetaLength;

            // vertex
            vertex.y = radius * Math.cos( segment );
            vertex.z = radius * Math.sin( segment );

            vertices.push( vertex.x, vertex.y, vertex.z );

            // normal
            normals.push( 1, 0, 0 );

            // uvs
            uv.x = ( vertices[ i ] / radius + 1 ) / 2;
            uv.y = ( vertices[ i + 1 ] / radius + 1 ) / 2;

            uvs.push( uv.x, uv.y );
            // colors.push(i === 0 ? otherColor : color);
        }

        // indices
        for ( i = 0; i < segments * arc; i ++ ) {

            indices.push( i, i + 1);
        }

        let count = indices.length;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('normal', new Float32Array(normals));
        mesh.addStream('uv', new Float32Array(uvs));
        mesh.addStream('index', new Uint32Array(indices));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);
        // mesh.addStream('subMesh', [{ 'start': 0, 'count': count1}, { 'start': count1, 'count': count - count1}]);
        mesh.submitToWebGL();

        return mesh;
    }

    ViewportGizmoUtil.createTorusHelperMesh = function(application, radius, tube, radialSegments, tubularSegments, arc)
    {
        let meshfactory = application.getMeshFactory();

        radius = radius || 1;
        tube = tube || 0.4;
        radialSegments = Math.floor( radialSegments ) || 8;
        tubularSegments = Math.floor( tubularSegments ) || 6;
        arc = arc || Math.PI * 2;


        let mesh = meshfactory.create();
        mesh.type = 'torus';

        // buffers
        let indices = [];
        let vertices = [];
        let normals = [];
        let uvs = [];

        // helper variables
        let center = new GASEngine.Vector3();
        let vertex = new GASEngine.Vector3();
        let normal = new GASEngine.Vector3();

        let j, i;

        // generate vertices, normals and uvs
        for (j = 0; j <= radialSegments; j ++) 
        {
            for (i = 0; i <= tubularSegments; i ++) 
            {
                let u = i / tubularSegments * arc;
                let v = j / radialSegments * Math.PI * 2;
                // vertex
                vertex.x = (radius + tube * Math.cos(v)) * Math.cos(u);
                vertex.y = (radius + tube * Math.cos(v)) * Math.sin(u);
                vertex.z = tube * Math.sin( v );

                vertices.push(vertex.x, vertex.y, vertex.z);

                // normal
                center.x = radius * Math.cos( u );
                center.y = radius * Math.sin( u );
                normal.subVectors(vertex, center).normalize();

                normals.push(normal.x, normal.y, normal.z);

                // uv
                uvs.push(i / tubularSegments);
                uvs.push(j / radialSegments);
            }
        }

        // generate indices
        for ( j = 1; j <= radialSegments; j ++ ) 
        {
            for ( i = 1; i <= tubularSegments; i ++ ) 
            {
                // indices
                var a = ( tubularSegments + 1 ) * j + i - 1;
                var b = ( tubularSegments + 1 ) * ( j - 1 ) + i - 1;
                var c = ( tubularSegments + 1 ) * ( j - 1 ) + i;
                var d = ( tubularSegments + 1 ) * j + i;

                // faces
                indices.push( a, b, d );
                // indices.push( b, c, d );
                indices.push( b, d, c );
            }
        }
        let count = indices.length;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('normal', new Float32Array(normals));
        mesh.addStream('uv', new Float32Array(uvs));
        mesh.addStream('index', new Uint32Array(indices));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);
        mesh.submitToWebGL();

        return mesh;
    }

    ViewportGizmoUtil.createBoxHelperMesh = function(application, size)
    {
        let meshfactory = application.getMeshFactory();

        size = size || 1;
        let mesh = meshfactory.create();
        mesh.type = 'box';

        let max = new GASEngine.Vector3(), min = new GASEngine.Vector3();
        max.set(size / 2, size / 2, size / 2);
        min.set(- size / 2, - size / 2, - size / 2);
        // buffers

        let vertices = 
        [
            max.x, max.y, max.z,
            min.x, max.y, max.z,
            min.x, min.y, max.z,
            max.x, min.y, max.z,
            max.x, max.y, min.z,
            min.x, max.y, min.z,
            min.x, min.y, min.z,
            max.x, min.y, min.z
        ];

        let indices = 
        [
            0, 1, 2, 0, 2, 3,
            4, 0, 3, 4, 3, 7,
            5, 4, 7, 5, 7, 6,
            1, 5, 6, 1, 6, 2,
            0, 4, 5, 0, 5, 1,
            3, 7, 6, 3, 6, 2
        ];

        let count = indices.length;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('index', new Uint32Array(indices));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);
        mesh.submitToWebGL();

        return mesh;
    }

    ViewportGizmoUtil.createSphereMesh = function(application, radius, widthSegments, heightSegments, phiStart, phiLength, thetaStart, thetaLength)
    {
        let meshfactory = application.getMeshFactory();

        radius = radius || 1;

        widthSegments = Math.max( 3, Math.floor( widthSegments ) || 100 );
        heightSegments = Math.max( 2, Math.floor( heightSegments ) || 100 );

        phiStart = phiStart !== undefined ? phiStart : 0;
        phiLength = phiLength !== undefined ? phiLength : Math.PI * 2;

        thetaStart = thetaStart !== undefined ? thetaStart : 0;
        thetaLength = thetaLength !== undefined ? thetaLength : Math.PI;

        var thetaEnd = thetaStart + thetaLength;
        var ix, iy;

        var index = 0;
        var grid = [];

        var vertex = new GASEngine.Vector3();
        var normal = new GASEngine.Vector3();

        // buffers
        var indices = [];
        var vertices = [];
        var normals = [];
        var uvs = [];

        // generate vertices, normals and uvs
        for ( iy = 0; iy <= heightSegments; iy ++ ) 
        {
            var verticesRow = [];
            var v = iy / heightSegments;

            for ( ix = 0; ix <= widthSegments; ix ++ ) 
            {
                var u = ix / widthSegments;

                // vertex
                vertex.x = - radius * Math.cos( phiStart + u * phiLength ) * Math.sin( thetaStart + v * thetaLength );
                vertex.y = radius * Math.cos( thetaStart + v * thetaLength );
                vertex.z = radius * Math.sin( phiStart + u * phiLength ) * Math.sin( thetaStart + v * thetaLength );
                vertices.push( vertex.x, vertex.y, vertex.z );

                // normal
                normal.set( vertex.x, vertex.y, vertex.z ).normalize();
                normals.push( normal.x, normal.y, normal.z );

                // uv
                uvs.push( u, 1 - v );
                verticesRow.push( index ++ );
            }

            grid.push( verticesRow );

        }

        // indices
        for ( iy = 0; iy < heightSegments; iy ++ ) 
        {
            for ( ix = 0; ix < widthSegments; ix ++ ) 
            {
                var a = grid[ iy ][ ix + 1 ];
                var b = grid[ iy ][ ix ];
                var c = grid[ iy + 1 ][ ix ];
                var d = grid[ iy + 1 ][ ix + 1 ];

                if ( iy !== 0 || thetaStart > 0 ) indices.push( a, b, d );
                if ( iy !== heightSegments - 1 || thetaEnd < Math.PI ) indices.push( b, c, d );
            }
        }

        var mesh = meshfactory.create();
        mesh.type = 'sphere';
        var count = indices.length;
        mesh.addStream('position', new Float32Array(vertices));
        mesh.addStream('normal', new Float32Array(normals));
        mesh.addStream('uv', new Float32Array(uvs));
        mesh.addStream('index', new Uint32Array(indices));
        mesh.addStream('subMesh', [{ 'start': 0, 'count': count}]);
        mesh.submitToWebGL();

        return mesh;
    }

    ViewportGizmoUtil.createGizmoEntity = function(application, entityName, shapeName, shapeInfo, translation, rotation, scale, color, depthTest, culling)
    {
        let componentFactory = application.getComponentFactory();
        let entity = application.createEditorEntity(entityName);

        // ----------------------- mesh renderer -----------------------
        let material = GASEngine.MaterialFactory.Instance.create('pureColor');
        material.depthTest = depthTest;
        material.culling = culling ? culling : material.culling;

        let meshRendererComponent = componentFactory.create('meshRenderer');
        meshRendererComponent.addMaterial(material);
        entity.addComponent(meshRendererComponent);
        if (color)
        {
            material.setPureColor(color);
        }
        else
        {
            material.visible = false;
        }

        // ----------------------- mesh filter -----------------------  
        let meshFilterComponent = componentFactory.create('meshFilter');
        meshFilterComponent.bbox = new GASEngine.AABB();
        if (shapeInfo.min && shapeInfo.max)
        {
            meshFilterComponent.bbox.max.copy(shapeInfo.min);
            meshFilterComponent.bbox.min.copy(shapeInfo.max);
        }

        let mesh = null;
        if (shapeName === 'cone')
        {
            let radiusTop = shapeInfo.radiusTop ? shapeInfo.radiusTop : 0;
            let radiusBottom = shapeInfo.radiusBottom ? shapeInfo.radiusBottom : GSIZE / 25;
            let height = shapeInfo.height ? shapeInfo.height : GSIZE / 6;
            mesh = this.createArrowHelperMesh(application, radiusTop, radiusBottom, height);
        }
        else if (shapeName === 'line')
        {
            let length = shapeInfo.length;
            mesh = this.createLineMesh(application, length);
        }
        else if (shapeName === 'plane')
        {
            let width = shapeInfo.width ? shapeInfo.width : GSIZE / 3;
            let height = shapeInfo.height ? shapeInfo.height : GSIZE / 3;
            let widthSegments = shapeInfo.widthSegments ? shapeInfo.widthSegments : undefined;
            let heightSegments = shapeInfo.heightSegments ? shapeInfo.heightSegments : undefined;
            let hasFlag = shapeInfo.hasFlag ? shapeInfo.hasFlag : undefined;
            mesh = this.createPlaneHelperMesh(application, width, height, widthSegments, heightSegments, hasFlag);
        }
        else if (shapeName === 'circle')
        {
            let radius = shapeInfo.radius ? shapeInfo.radius : GSIZE;
            let segments = shapeInfo.segments ? shapeInfo.segments : 100;
            let halfFlag = shapeInfo.halfFlag ? shapeInfo.halfFlag : undefined;
            let thetaStart = shapeInfo.thetaStart ? shapeInfo.thetaStart : undefined;
            let thetaLength = shapeInfo.thetaLength ? shapeInfo.thetaLength : undefined;
            mesh = this.createCircleHelperMesh(application, radius, segments, halfFlag, thetaStart, thetaLength);   
        }
        else if (shapeName === 'box')
        {
            let size = shapeInfo.size ? shapeInfo.size : GSIZE / 10;
            mesh = this.createBoxHelperMesh(application, size);
        }
        else if (shapeName === 'torus')
        {
            let radius = shapeInfo.radius ? shapeInfo.radius : GSIZE;
            let tube = shapeInfo.tube ? shapeInfo.tube : GSIZE / 15;
            let radialSegments = shapeInfo.radialSegments ? shapeInfo.radialSegments : 4;
            let tubularSegments = shapeInfo.tubularSegments ? shapeInfo.tubularSegments : 24;
            let arc = shapeInfo.arc ? shapeInfo.arc : undefined;
            mesh = this.createTorusHelperMesh(application, radius, tube, radialSegments, tubularSegments, arc);
        }
        else if (shapeName === 'sphere')
        {
            let radius = shapeInfo.radius ? shapeInfo.radius : 0.99 * GSIZE;
            let widthSegments = shapeInfo.widthSegments ? shapeInfo.widthSegments : undefined;
            let heightSegments = shapeInfo.heightSegments ? shapeInfo.heightSegments : undefined;
            let phiStart = shapeInfo.phiStart ? shapeInfo.phiStart : undefined;
            let phiLength = shapeInfo.phiLength ? shapeInfo.phiLength : undefined;
            let thetaStart = shapeInfo.thetaStart ? shapeInfo.thetaStart : undefined;
            let thetaLength = shapeInfo.thetaLength ? shapeInfo.thetaLength : undefined;
            mesh = this.createSphereMesh(application, radius, widthSegments, heightSegments, phiStart, phiLength, thetaStart, thetaLength);
        }
        meshFilterComponent.setMesh(mesh);
        entity.addComponent(meshFilterComponent);

        // ----------------------- trs setting -----------------------
        if (translation !== undefined)
        {
            entity.setLocalTranslation(translation);
        }

        if (rotation !== undefined)
        {
            entity.setLocalRotation(rotation);
        }

        if (scale !== undefined)
        {
            entity.setLocalScale(scale);
        }

        return entity;
    }

    // -------------------------------------------- viewport Item --------------------------------------------

    let GizmoItem = function(name, application)
    {
        mgs.Events.call(this);

        this._application = application;
        this._root = this._application.createEditorEntity(GIZMO_PREFIX + name);

        this._name = name;
        this._childInfos = {};
    };
    mgs.classInherit(GizmoItem, mgs.Events);

    GizmoItem.prototype.update = function(delta)
    { 
    }; 

    GizmoItem.prototype.getName = function()
    { 
        return this._name; 
    }; 

    GizmoItem.prototype.getRoot = function()
    { 
        return this._root; 
    };   

    GizmoItem.prototype.addChildInfo = function(name, displayEntities, pickerEntities, color, highlightColor)
    {
        for (let i = 0;i < displayEntities.length;i ++)
        {
            this._root.addChild(displayEntities[i]);
        }

        for (let i = 0;i < pickerEntities.length;i ++)
        {
            this._root.addChild(pickerEntities[i]);
        }

        this._childInfos[name] = 
        { 
            name: name, rootName: this._name, displayEntities: displayEntities, pickerEntities: pickerEntities,
            originColor: color, currentColor: color, highlightColor: highlightColor
        };
    };

    GizmoItem.prototype.getChildInfos = function()
    {
        return this._childInfos;
    };

    GizmoItem.prototype.getChildInfo = function(name)
    {
        for (let key in this._childInfos)
        {
            let childInfo = this._childInfos[key];
            if (childInfo.name === name)
            {
                return childInfo;
            }
        }

        return null;
    };

    GizmoItem.prototype.getChildByPickEntity = function(entity)
    {
        for (let key in this._childInfos)
        {
            let childInfo = this._childInfos[key];
            for (let index = 0;index < childInfo.pickerEntities.length;index ++)
            {
                if (entity.uniqueID === childInfo.pickerEntities[index].uniqueID)
                {
                    return childInfo;
                }
            }
        }

        return null;
    };

    GizmoItem.prototype.getChildByPickerEntity = function(entity)
    {
        for (let key in this._childInfos)
        {
            let childInfo = this._childInfos[key];
            for (let index in childInfo.pickerEntities.length)
            {
                let pickerEntity = childInfo.pickerEntities[index];
                if (entity.uniqueID === pickerEntity.uniqueID)
                {
                    return childInfo;
                }
            }
        }
    };

    GizmoItem.prototype.pickChildInfo = function()
    {
        let entities = [];
        for (let key in this._childInfos)
        {
            let childInfo = this._childInfos[key];
            entities.push(...childInfo.pickerEntities);
        }

        let canvas = this._application.getWebGLDevice().canvas;
        let camera = this._application.getCurrentCamera();

        var mousePos = mgs.editor.getMousePos();
        var pointerPos = GASEngine.Utilities.getPointerPosition1(mousePos, canvas);

        var raycaster = new GASEngine.Raycaster();
        raycaster.setFromCamera(pointerPos, camera);

        var intersects = raycaster.intersectObjects(entities);
        if (intersects.length > 0)
        {
            let entity = intersects[0].object;
            return this.getChildByPickEntity(entity);
        }

        return null;
    };

    GizmoItem.prototype._setChildInfoColor = function(childInfo, color)
    { 
        childInfo.currentColor = color;

        for (let i = 0;i < childInfo.displayEntities.length;i ++)
        {
            let entity = childInfo.displayEntities[i];
            let meshRenderer = entity.getComponent('meshRenderer');
            let materials = meshRenderer.getMaterials();
            materials[0].setPureColor(childInfo.currentColor);
        }
    };

    GizmoItem.prototype._resumeChildInfoColor = function(childInfo)
    { 
        childInfo.currentColor = childInfo.originColor;

        for (let i = 0;i < childInfo.displayEntities.length;i ++)
        {
            let entity = childInfo.displayEntities[i];
            let meshRenderer = entity.getComponent('meshRenderer');
            let materials = meshRenderer.getMaterials();
            materials[0].setPureColor(childInfo.currentColor);
        }
    };

    GizmoItem.prototype.setChildInfoHover = function(name)
    { 
        for (let key in this._childInfos)
        {
            let childInfo = this._childInfos[key];

            if (childInfo.name === name)
            {
                this._setChildInfoColor(childInfo, childInfo.highlightColor ? childInfo.highlightColor : HOVER_COLOR);
            }
            else
            {
                this._resumeChildInfoColor(childInfo);
            }
        }
    };

    GizmoItem.prototype.Enable = function(isEnable)
    { 
        this._root.enable = isEnable;
        if (isEnable)
        {
            
        }
    };

    let GizmoItem_Translate = function(application)
    {
        mgs.GizmoItem.call(this, 'translate', application);

        // x axis
        this.createAxis('X', RED_COLOR, 
        new GASEngine.Vector3(GSIZE, 0, 0), new GASEngine.Euler(0, 0, - Math.PI / 2),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0),
        new GASEngine.Vector3(GSIZE * 0.6, 0, 0), new GASEngine.Euler(0, 0, -Math.PI / 2));

        // y axis
        this.createAxis('Y', GREEN_COLOR, 
        new GASEngine.Vector3(0, GSIZE, 0), new GASEngine.Euler(0, 0, 0),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, Math.PI / 2),
        new GASEngine.Vector3(0, GSIZE * 0.6, 0), new GASEngine.Euler(0, 0, 0));

        // z axis
        this.createAxis('Z', BLUE_COLOR, 
        new GASEngine.Vector3(0, 0, GSIZE), new GASEngine.Euler(Math.PI / 2, 0, 0),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, - Math.PI / 2, 0),
        new GASEngine.Vector3(0, 0, GSIZE * 0.6), new GASEngine.Euler(Math.PI / 2, 0, 0));

        // xy plane
        this.createPlane('XY', XY_COLOR, new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0));

        // yz plane
        this.createPlane('YZ', YZ_COLOR, new GASEngine.Vector3(0, 0, GSIZE / 3), new GASEngine.Euler(0, Math.PI / 2, 0));
        
        // zx plane
        this.createPlane('ZX', ZX_COLOR, new GASEngine.Vector3(0, 0, GSIZE / 3), new GASEngine.Euler(-Math.PI / 2, 0, 0));
    };
    mgs.classInherit(GizmoItem_Translate, mgs.GizmoItem);

    GizmoItem_Translate.prototype.createAxis = function(axisName, color, coreTranslate, coreRotate, lineTranslate, lineRotate, pickerTranslate, pickerRotate)
    {
        let pickerConeFactor = 7.5;
        let pickerConeRadiusTop = GSIZE / pickerConeFactor;

        // displayEntities
        let coneEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + axisName + 'Cone', 
            'cone',
            { radiusTop: 0, radiusBottom: GSIZE / 25, height: GSIZE / 6 },
            coreTranslate,
            coreRotate,
            new GASEngine.Vector3(1, 1, 1),
            color,
            false
        );

        let lineEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + axisName + 'Line', 
            'line',
            { length: GSIZE },
            lineTranslate,
            lineRotate,
            new GASEngine.Vector3(1, 1, 1),
            color,
            false
        )
        
        // pickerEntities
        let pickerEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + axisName + 'Picker', 
            'cone',
            { 
                radiusTop: pickerConeRadiusTop, radiusBottom: 0, height: GSIZE, 
                min: new GASEngine.Vector3(-pickerConeRadiusTop, -GSIZE, -pickerConeRadiusTop), 
                max: new GASEngine.Vector3(pickerConeRadiusTop, GSIZE, pickerConeRadiusTop)
            },
            pickerTranslate,
            pickerRotate,
            new GASEngine.Vector3(1, 1, 1),
            // WHITE_COLOR,
            // false
        )

        this.addChildInfo(axisName, [coneEntity, lineEntity], [pickerEntity], color);
    };
   
    GizmoItem_Translate.prototype.createPlane = function(planeName, color, translate, rotation)
    {
        let pickerPlaneFactor = 3;
        let pickerPlaneSize = GSIZE / pickerPlaneFactor;

        let planeEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + planeName + 'Plane', 
            'plane',
            { width: pickerPlaneSize, height: pickerPlaneSize },
            translate,
            rotation,
            new GASEngine.Vector3(1, 1, 1),
            color,
            false
        );

        
        let pickerEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + planeName + 'Picker', 
            'plane',
            { 
                width: pickerPlaneSize, height: pickerPlaneSize,
                min: new GASEngine.Vector3(0, 0, 0), 
                max: new GASEngine.Vector3(pickerPlaneSize, pickerPlaneSize, pickerPlaneSize)
            },
            translate,
            rotation,
            new GASEngine.Vector3(1, 1, 1),
            // WHITE_COLOR,
            // false
        );

        this.addChildInfo(planeName, [planeEntity], [pickerEntity], color);
    };

    
    let GizmoItem_Rotate = function(application)
    {
        mgs.GizmoItem.call(this, 'rotate', application);

        // XYZE
        this.createSphere('XYZE', SPHERE_COLOR, SPHERE_HIGHLIGHT_COLOR);

        // x ring
        this.createCircle('X', RED_COLOR, 
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0), new GASEngine.Euler(1, 1, 1),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, Math.PI / 2, 0), new GASEngine.Euler(1, 1, 1));

        // y ring
        this.createCircle('Y', GREEN_COLOR, 
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, Math.PI / 2), new GASEngine.Euler(1, 1, 1),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(Math.PI / 2, 0, 0), new GASEngine.Euler(1, 1, 1));

        // z ring
        this.createCircle('Z', BLUE_COLOR, 
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, Math.PI / 2, 0), new GASEngine.Euler(1, 1, 1),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0), new GASEngine.Euler(1, 1, 1));

        // E ring
        this.createCircle('E', WHITE_COLOR, 
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0), new GASEngine.Euler(1.25, 1.25, 1.25,),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0), new GASEngine.Euler(1.25, 1.25, 1.25,));

        // gray ring
        this.createCircle('E1', GRAY_COLOR, 
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0), new GASEngine.Euler(1, 1, 1),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0), new GASEngine.Euler(1, 1, 1), true);
    };
    mgs.classInherit(GizmoItem_Rotate, mgs.GizmoItem);

    GizmoItem_Rotate.prototype.update = function(delta)
    {
        let camera = this._application.getCurrentCamera();
        let camMatrix = camera.getWorldMatrix();
        camMatrix.decompose(cameraPosition, cameraQuaternion, cameraScale);

        let worldPosition = this._root.getWorldTranslation();

        dir.copy(cameraPosition).sub(worldPosition);
        // dir.applyEuler(new GASEngine.Euler(0, Math.PI / 2, 0));
 
        lookAt.copy(worldPosition).add(dir);

        matrix.lookAt(worldPosition, lookAt, unitY);
        gizmoQuaternion.setFromRotationMatrix(matrix);
        gizmoQuaternion.multiply(quaternionOffset);
        gizmoRotation.setFromQuaternion(gizmoQuaternion);

        let e1Info = this.getChildInfo('E1');
        let eInfo = this.getChildInfo('E');

        // display
        let entities = [];
        entities.push(...e1Info.displayEntities);
        entities.push(...eInfo.displayEntities);

        for (let i = 0;i < entities.length;i ++)
        {
            entities[i].setWorldRotation(gizmoRotation);
        }

        // picker
        matrix.lookAt(worldPosition, cameraPosition, unitY);
        gizmoQuaternion.setFromRotationMatrix(matrix);
        gizmoRotation.setFromQuaternion(gizmoQuaternion);

        for (let i = 0;i < eInfo.pickerEntities.length;i ++)
        {
            eInfo.pickerEntities[i].setWorldRotation(gizmoRotation);
        }
    }; 

    GizmoItem_Rotate.prototype.createCircle = function(circleName, color, translate, rotation, scale, torusTranslate, torusRotation, torusScale, disablePick)
    {
        let circleEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + circleName + 'Circle', 
            'circle',
            { radius: GSIZE, segments: 100 },
            translate,
            rotation,
            scale,
            color,
            true
        );

        let torusEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + circleName + 'Picker', 
            'torus',
            { 
                radius: GSIZE,
                tube: GSIZE / 15,
                radialSegments: 4,
                tubularSegments: 24,
                min: new GASEngine.Vector3((1 + 1/15) * GSIZE, (1 + 1/15) * GSIZE, 1/15 * GSIZE), 
                max: new GASEngine.Vector3(- (1 + 1/15) * GSIZE, - (1 + 1/15) * GSIZE, - 1/15 * GSIZE)
            },
            torusTranslate,
            torusRotation,
            torusScale,
            // WHITE_COLOR,
            // true
        );

        let pickerEntities = disablePick ? [] : [torusEntity];

        this.addChildInfo(circleName, [circleEntity], pickerEntities, color);
    };

    GizmoItem_Rotate.prototype.createSphere = function(sphereName, color, sphereHighlightColor, translate, rotation, scale)
    {
        let sphereSize = 0.99 * GSIZE;

        let sphereEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + sphereName + 'Sphere', 
            'sphere',
            { 
                radius: sphereSize,
                min: new GASEngine.Vector3(sphereSize, sphereSize, sphereSize), 
                max: new GASEngine.Vector3(- sphereSize, - sphereSize, - sphereSize), 
            },
            translate,
            rotation,
            scale,
            color,
            true,
            GASEngine.Material.CullingOnCW
        );

        let pickerEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + sphereName + 'Picker', 
            'sphere',
            { 
                radius: sphereSize,
                min: new GASEngine.Vector3(sphereSize, sphereSize, sphereSize), 
                max: new GASEngine.Vector3(- sphereSize, - sphereSize, - sphereSize), 
            },
            translate,
            rotation,
            scale,
            // WHITE_COLOR,
            // true
        );

        this.addChildInfo(sphereName, [sphereEntity], [pickerEntity], color, sphereHighlightColor);
    };

      
    let GizmoItem_Scale = function(application)
    {
        mgs.GizmoItem.call(this, 'scale', application);

        // x axis
        this.createAxis('X', RED_COLOR, 
        new GASEngine.Vector3(GSIZE, 0, 0), new GASEngine.Euler(0, 0, 0),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0),
        new GASEngine.Vector3(GSIZE * 0.6, 0, 0), new GASEngine.Euler(0, 0, -Math.PI / 2));

        // y axis
        this.createAxis('Y', GREEN_COLOR, 
        new GASEngine.Vector3(0, GSIZE, 0), new GASEngine.Euler(0, 0, 0),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, Math.PI / 2),
        new GASEngine.Vector3(0, GSIZE * 0.6, 0), new GASEngine.Euler(0, 0, 0));

        // z axis
        this.createAxis('Z', BLUE_COLOR, 
        new GASEngine.Vector3(0, 0, GSIZE), new GASEngine.Euler(0, 0, 0),
        new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, -Math.PI / 2, 0),
        new GASEngine.Vector3(0, 0, GSIZE * 0.6), new GASEngine.Euler( Math.PI / 2, 0, 0));

        // xy plane
        this.createPlane('XY', XY_COLOR, new GASEngine.Vector3(0, 0, 0), new GASEngine.Euler(0, 0, 0));

        // yz plane
        this.createPlane('YZ', YZ_COLOR, new GASEngine.Vector3(0, 0, GSIZE / 3), new GASEngine.Euler(0, Math.PI / 2, 0));
        
        // zx plane
        this.createPlane('ZX', ZX_COLOR, new GASEngine.Vector3(0, 0, GSIZE / 3), new GASEngine.Euler(-Math.PI / 2, 0, 0));
    };
    mgs.classInherit(GizmoItem_Scale, mgs.GizmoItem);

    GizmoItem_Scale.prototype.createAxis = function(axisName, color, boxTranslate, boxRotate, lineTranslate, lineRotate, pickerTranslate, pickerRotate)
    {
        let pickerConeFactor = 7.5;
        let pickerConeRadiusTop = GSIZE / pickerConeFactor;

        // displayEntities
        let boxEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + axisName + 'Box', 
            'box',
            { size: GSIZE/10 },
            boxTranslate,
            boxRotate,
            new GASEngine.Vector3(1, 1, 1),
            color,
            false
        );

        let lineEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + axisName + 'Line', 
            'line',
            { length: GSIZE },
            lineTranslate,
            lineRotate,
            new GASEngine.Vector3(1, 1, 1),
            color,
            false
        )
        
        // pickerEntities
        let pickerEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + axisName + 'Picker', 
            'cone',
            { 
                radiusTop: pickerConeRadiusTop, radiusBottom: 0, height: GSIZE, 
                min: new GASEngine.Vector3(-pickerConeRadiusTop, -GSIZE, -pickerConeRadiusTop), 
                max: new GASEngine.Vector3(pickerConeRadiusTop, GSIZE, pickerConeRadiusTop)
            },
            pickerTranslate,
            pickerRotate,
            new GASEngine.Vector3(1, 1, 1),
            // WHITE_COLOR,
            // false
        )

        this.addChildInfo(axisName, [boxEntity, lineEntity], [pickerEntity], color);
    };
   
    GizmoItem_Scale.prototype.createPlane = function(planeName, color, translate, rotation)
    {
        let pickerPlaneFactor = 3;
        let pickerPlaneSize = GSIZE / pickerPlaneFactor;

        let planeEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + planeName + 'Plane', 
            'plane',
            { width: pickerPlaneSize, height: pickerPlaneSize },
            translate,
            rotation,
            new GASEngine.Vector3(1, 1, 1),
            color,
            false
        );

        
        let pickerEntity = ViewportGizmoUtil.createGizmoEntity
        (
            this._application, GIZMO_PREFIX + this._name + '_Display_' + planeName + 'Picker', 
            'plane',
            { 
                width: pickerPlaneSize, height: pickerPlaneSize,
                min: new GASEngine.Vector3(0, 0, 0), 
                max: new GASEngine.Vector3(pickerPlaneSize, pickerPlaneSize, pickerPlaneSize)
            },
            translate,
            rotation,
            new GASEngine.Vector3(1, 1, 1),
            // WHITE_COLOR,
            // false
        );

        this.addChildInfo(planeName, [planeEntity], [pickerEntity], color);
    };

    // -------------------------------------------- viewport gizmo --------------------------------------------

    let ViewportGizmo = function(application)
    {
        mgs.Object.call(this);

        this._application = application;
        this._createGizmos();
    };
    mgs.classInherit(ViewportGizmo, mgs.Object);

    ViewportGizmo.prototype.destroy = function()
    {
    };

    ViewportGizmo.prototype.update = function(delta)
    {
        for (let key in this._gizmoItemMap)
        {
            let gizmoItem = this._gizmoItemMap[key];
            gizmoItem.update(delta);
        }
    };

    ViewportGizmo.prototype._createGizmos = function()
    {
        this._gizmoItemMap = {};

        // gizmo root
        this._gizmoRoot = this._application.createEditorEntity(GIZMO_PREFIX + 'Root');
        this._application.getEditorRoot().addChild(this._gizmoRoot);

        // ----- translation ----- //
        {
            let item = new mgs.GizmoItem_Translate(this._application);
            this._gizmoRoot.addChild(item.getRoot());
            this._gizmoItemMap[item.getName()] = item;
        }
        
        // ----- rotation ----- //
        {
            let item = new mgs.GizmoItem_Rotate(this._application);
            this._gizmoRoot.addChild(item.getRoot());
            this._gizmoItemMap[item.getName()] = item;
        }

        // ----- scale ----- //
        {
            let item = new mgs.GizmoItem_Scale(this._application);
            this._gizmoRoot.addChild(item.getRoot());
            this._gizmoItemMap[item.getName()] = item;
        }

        this.enableGizmoItem(null);
    };

    ViewportGizmo.prototype.pickGizmoChildInfo = function(mode)
    {
        let gizmoItem = this._gizmoItemMap[mode];
        if (gizmoItem !== undefined)
        {
            return gizmoItem.pickChildInfo();
        }

        return null;
    };

    ViewportGizmo.prototype.getGizmoItem = function(mode)
    {
        let gizmoItem = this._gizmoItemMap[mode];
        return gizmoItem;
    };

    ViewportGizmo.prototype.enableGizmoItem = function(mode)
    {
        for (let key in this._gizmoItemMap)
        {
            let gizmoItem = this._gizmoItemMap[key];
            if (key === mode)
            {
                gizmoItem.Enable(true);
            }
            else
            {
                gizmoItem.Enable(false);
            }
        }
    };

    ViewportGizmo.prototype.setGizmoTRS = function(worldTranslation, worldQuaternion, worldScale)
    {
        tmpRotation.setFromQuaternion(worldQuaternion);
        this._gizmoRoot.setWorldTranslation(worldTranslation);
        this._gizmoRoot.setWorldRotation(tmpRotation);
        this._gizmoRoot.setWorldScale(worldScale);
    };

    ViewportGizmo.prototype.getRotateRadius = function()
    {
        tempScale.copy(this._gizmoRoot.getWorldScale());
        return tempScale.x * GSIZE;
    };
}());