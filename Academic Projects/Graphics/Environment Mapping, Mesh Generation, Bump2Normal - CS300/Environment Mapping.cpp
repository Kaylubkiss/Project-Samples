/******************************************************************************/
/*!
\fn     void ComputeMirrorCamMats()
\brief
        Compute the view/projection and other related matrices for mirror camera.
*/
/******************************************************************************/
void ComputeMirrorCamMats()
{
    if (mainCam.moved)
    {
        /*  Computing position of user camera in mirror frame */
        Vec3 mainCamMirrorFrame = Vec3(Translate(-mirrorTranslate) * Vec4(mainCam.pos, 1.0));

        /*  If user camera is behind mirror, then mirror is not visible and no need to compute anything */
        if (mainCamMirrorFrame.z <= 0)
        {
            mirrorVisible = false;
            return;
        }
        else
            mirrorVisible = true;


        /*  In mirror frame, mirror camera position is defined as (x, y, -z) in which (x, y, z) is the
            user camera position in mirror frame.
            We also need to compute mirrorCam.pos, mirrorCam.upVec, mirrorCam.lookAt are defined in world
            frame to compute mirror cam's view matrix.
            function to compute mirrorCamViewMat
        */
        Vec3 mirrorCamMirrorFrame = Vec3(mainCamMirrorFrame.x, mainCamMirrorFrame.y, -mainCamMirrorFrame.z);

        mirrorCam.pos = Vec3(Translate(mirrorTranslate) * Vec4(mirrorCamMirrorFrame, 1.0));
        mirrorCam.upVec = BASIS[Y];
        mirrorCam.lookAt = Vec3(Translate(mirrorTranslate) * Vec4(0, 0, 0, 1));

        mirrorCamViewMat = LookAt(mirrorCam.pos, mirrorCam.lookAt, mirrorCam.upVec);

        ComputeObjMVMats(mirrorCamMVMat, mirrorCamNormalMVMat, mirrorCamViewMat);


          //@TODO Assignment5
          /*  Compute mirror camera projection matrix */
          /*  In mirror frame, the mirror camera view direction is towards the center of the mirror,
              which is the origin of this frame.

              mirrorCam.nearPlane is computed as the smallest projected length along the view direction
              of the vectors from the mirror camera to the midpoints of the mirror left, right, bottom
              and top edges. These midpoints are pre-defined in mirror frame as (0.5, 0, 0) [left],
              (-0.5, 0, 0) [right], (0, -0.5, 0) [bottom] and (0, 0.5, 0) [top].

              mirrorCam.farPlane is set to INFINITY.

              leftPlane, rightPlane, bottomPlane, topPlane are computed based on the intersections between
              the near plane and the vectors from camera to the left, right, bottom and top edge midpoints.
              After you've done with this computation, you would see the reflected image with a quite narrow
              view angle (which is physically correct). However, in order to widen the view angle, we would
              multiply all leftPlane, rightPlane, bottomPlane, topPlane by 2.
          */
          /*  ... */
          /*  Compute mirror cam's projection matrix */
        Vec3 mirror_l = Vec3(0.5, 0, 0);
        Vec3 mirror_r = Vec3(-0.5, 0, 0);
        
        Vec3 mirror_b = Vec3(0, -0.5, 0);
        Vec3 mirror_t = Vec3(0, 0.5, 0);

        Vec3 mirror_viewNormal = Normalize(-mirrorCamMirrorFrame);

        Vec3 mEyeP(mirrorCamMirrorFrame); 

        Vec3 leftLineDir = Normalize(mirror_l - mEyeP);
        Vec3 rightLineDir = Normalize(mirror_r - mEyeP);
        Vec3 bottomLineDir = Normalize(mirror_b - mEyeP);
        Vec3 topLineDir = Normalize(mirror_t - mEyeP);


        float dis_left = Dot(mirror_viewNormal, leftLineDir);
        float dis_right = Dot(mirror_viewNormal, rightLineDir);

        float dis_bottom = Dot(mirror_viewNormal, bottomLineDir);
        float dis_top = Dot(mirror_viewNormal, topLineDir);

        mirrorCam.nearPlane = std::min(std::min(dis_left, dis_right), std::min(dis_bottom, dis_top));


        Vec3 nearPoint = mirror_viewNormal * mirrorCam.nearPlane + (mEyeP);

        Vec3 nToM_l = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_l, leftLineDir);
        Vec3 nToM_r = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_r, rightLineDir);
        Vec3 nToM_b = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_b, bottomLineDir);
        Vec3 nToM_t = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_t, topLineDir);

        mirrorCam.leftPlane = -2.f * Distance(nearPoint, nToM_l);
        mirrorCam.rightPlane = 2.f * Distance(nearPoint, nToM_r);
        mirrorCam.bottomPlane = -2.f * Distance(nearPoint, nToM_b);
        mirrorCam.topPlane = 2.f * Distance(nearPoint, nToM_t);

        mirrorCamProjMat = Frustum(mirrorCam.leftPlane, mirrorCam.rightPlane, mirrorCam.bottomPlane, 
        mirrorCam.topPlane, mirrorCam.nearPlane, mirrorCam.farPlane);
    }
}

/******************************************************************************/
/*!
\fn     void ComputeSphereCamMats()
\brief
        Compute the view/projection and other related matrices for sphere camera.
*/
/******************************************************************************/
void ComputeSphereCamMats()
{
    //@TODO Assignment5
    /*  Compute the lookAt positions for the 6 faces of the sphere cubemap. 
        The sphere camera is at spherePos.
        The front and back faces are -z and +z.
        The left and right faces are -x and +x.
        The bottom and top faces are -y and +y.
    */
    Vec3 lookAt[CubeFaceID::NUM_FACES];
    /*  ... */

    //IDEA: store the lookAts and upVecs  in order of the textures...
    // 
    //+x,-x
    lookAt[0] = spherePos + Vec3(1, 0, 0);
    lookAt[1] = spherePos + Vec3(-1, 0, 0);
    
    //+y,-y
    lookAt[2] = spherePos + Vec3(0, 1, 0);
    lookAt[3] = spherePos + Vec3(0, -1, 0);

    //+z,-z
    lookAt[4] = spherePos + Vec3(0, 0, 1);
    lookAt[5] = spherePos + Vec3(0, 0, -1);


    //@TODO Assignment5
    /*  Compute upVec for the 6 faces of the sphere cubemap.
        The front, back, left and right faces are flipped upside-down.
        The top face's upVec is pointing forward and the bottom face's
        upVec is pointing backward.
    */
    Vec3 upVec[CubeFaceID::NUM_FACES];
    /*  ... */


    upVec[0] = Vec3(0, -1, 0);
    upVec[1] = Vec3(0, -1, 0);

    upVec[2] = Vec3(0, 0, 1);
    upVec[3] = Vec3(0, 0, -1);

    upVec[4] = Vec3(0, -1, 0);
    upVec[5] = Vec3(0, -1, 0);


    //upVec[2] = Vec3(0, 1, 0);    // +y - Pointing upward along positive y-axis
    //upVec[3] = Vec3(0, -1, 0);   // -y - Pointing downward along negative y-axis

    for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
    {
        sphereCamViewMat[f] = LookAt(spherePos, lookAt[f], upVec[f]);
        ComputeObjMVMats(sphereCamMVMat[f], sphereCamNormalMVMat[f], sphereCamViewMat[f]);
    }


    //@TODO Assignment5
    /*  Use Perspective function to ompute the projection matrix sphereCamProjMat so that 
        from the camera position at the cube center, we see a complete face of the cube.
        The near plane distance is 0.01f. The far plane distance is equal to mainCam's farPlane.
    */
    /*  ... */
    sphereCamProjMat = Perspective(HALF_PI, 1.f, .01f, mainCam.farPlane);
}