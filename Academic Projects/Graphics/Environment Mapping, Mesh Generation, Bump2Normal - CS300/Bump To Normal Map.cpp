/******************************************************************************/
/*!
\fn     void SetUpBaseBumpNormalTextures()
\brief
        Set up the bump map and normal map for normal mapping and parallax mapping.
*/
/******************************************************************************/
void SetUpBaseBumpNormalTextures()
{
    unsigned char *bumpImgData, *normalImgData;
    int imgWidth, imgHeight, numComponents;

    /*  Load bump image */
    if (ReadImageFile(bumpTexFile, &bumpImgData, &imgWidth, &imgHeight, &numComponents) == 0)
    {
        std::cerr << "Reading " << bumpTexFile << " failed.\n";
        exit(1);
    }

    /*  Create normal image */
    normalImgData = (unsigned char *)malloc(imgWidth * imgHeight * 3 * sizeof(unsigned char));

    Bump2Normal(bumpImgData, normalImgData, imgWidth, imgHeight);

    /*  Generate texture ID for bump image and copy it to GPU */
    /*  Bump image will be used to compute the offset in parallax mapping */
    glGenTextures(1, &bumpTexID);
    glBindTexture(GL_TEXTURE_2D, bumpTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, imgWidth, imgHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bumpImgData);

    free(bumpImgData);

    /*  Generate texture mipmaps. */
    glGenerateMipmap(GL_TEXTURE_2D);
    /*  Set up texture behaviors */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    /*  Generate texture ID for normal image and copy it to GPU */
    glGenTextures(1, &normalTexID);
    glBindTexture(GL_TEXTURE_2D, normalTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imgWidth, imgHeight, 0,
        GL_RGB, GL_UNSIGNED_BYTE, normalImgData);

    free(normalImgData);

    /*  Generate texture mipmaps. */
    glGenerateMipmap(GL_TEXTURE_2D);

    /*  Set up texture behaviors */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}


/******************************************************************************/
/*!
\fn     void Bump2Normal(   const unsigned char *bumpImg, 
                            unsigned char *normalImg, 
                            int width, int height)
\brief
        Compute normal map from bump map.
\param  bumpImg
        Given 1D bump map.
\param  normalImg
        3D normal map to be computed.
\param  width
        Width of the texture map.
\param  height
        Height of the texture map.
*/
/******************************************************************************/
void Bump2Normal(const unsigned char *bumpImg, unsigned char *normalImg, int width, int height)
{
    /*  Compute the normal map from bump map using the formula in the lecture note, in which a is 40.
        For the leftmost/rightmost columns or bottom/top rows, the formula should use the 
        two leftmost/rightmost columns or two bottom/top rows and 0.5*a.
        Note that the formula in the lecture note assumes the bump/height values are within [0, 1].
        But our bumpImg values range from 0 to 255.
    */
    float a = 40.0f;

    float alpha_delta_bt = a;
    float alpha_delta_lr = a;

    for (int i = 0; i < height; i++) {

        for (int j = 0; j < width; j++) {

            int idx = i * width + j;

            //leftmost? or just a left?
            float Bi_minus_1;
            if (j == 0)
            { 
                alpha_delta_lr = a * .5f;
                Bi_minus_1 = static_cast<float>(bumpImg[idx]) / 255.f;
            }
            else 
            { 
                Bi_minus_1 = static_cast<float>(bumpImg[idx - 1]) / 255.f;
            }

            //rightmost? or just right?
            float Bi_plus_1;
            if (j == width - 1) 
            {
                alpha_delta_lr = a * .5f;
                Bi_plus_1 = static_cast<float>(bumpImg[idx]) / 255.f ;
            }
            else 
            { 
                Bi_plus_1 = static_cast<float>(bumpImg[idx + 1]) / 255.f ;
            }


            //top most? or just a top?
            float Bj_plus_1;
            if (i == height-1) 
            { 
                alpha_delta_bt = a * .5f;
                Bj_plus_1 = static_cast<float>(bumpImg[idx]) / 255.f;
            }
            else 
            { 
                Bj_plus_1 = static_cast<float>(bumpImg[idx + width]) / 255.f ;
            }

            float Bj_minus_1;
            if (i == 0) 
            {
                alpha_delta_bt = a * .5f;
                Bj_minus_1 = static_cast<float>(bumpImg[idx]) / 255.f ;
            }
            else 
            { 
                Bj_minus_1 = static_cast<float>(bumpImg[idx - width]) / 255.f ;
            } //bottom most? or just bottom.

            float Bu_z = (Bi_plus_1 - Bi_minus_1) * alpha_delta_lr;
            float Bv_z = (Bj_plus_1 - Bj_minus_1) * alpha_delta_bt;

    
            Vec3 Bu = Vec3(1, 0, Bu_z);
            Vec3 Bv = Vec3(0, 1, Bv_z);
        
            Vec3 pert_normal = glm::normalize(glm::cross(Bu,Bv));

        
            normalImg[idx * 3] = static_cast<unsigned char>((pert_normal.y  + 1.0f) * 127.5f); //r
            normalImg[idx * 3 + 1] = static_cast<unsigned char>((pert_normal.x + 1.0f) * 127.5f);//g
            normalImg[idx * 3 + 2] = static_cast<unsigned char>((pert_normal.z  + 1.0f) * 127.5f);//b

            alpha_delta_bt = a;
            alpha_delta_lr = a;
        }
    }
}

