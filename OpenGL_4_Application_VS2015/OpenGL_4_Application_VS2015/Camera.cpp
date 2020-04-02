//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"
#include <iostream>
namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
		this->cameraHeight = cameraPosition.y;
    }
    

    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
    }

	glm::vec3 Camera::getCameraTarget()
	{
		return cameraTarget;
	}
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
				//std::cout << cameraPosition.x << cameraPosition.y << cameraPosition.z << "\n";
				//cameraPosition.z -= speed;
				cameraPosition.y = cameraHeight;
				/*if (cameraPosition.y != cameraHeight) {
					cameraPosition.y = cameraHeight;
				}*/
                break;
                
            case MOVE_BACKWARD:
               /* cameraPosition -= cameraDirection * speed;
				if (cameraPosition.y != cameraHeight) {
				}*/
				cameraPosition -= cameraDirection * speed;
				cameraPosition.y = cameraHeight;
				//cameraPosition.z += speed;
                
				break;	
                
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                break;

			case MOVE_UP:
				cameraPosition.y += speed;
				cameraHeight += speed;
				break;

			case MOVE_DOWN:
				cameraPosition.y -= speed;
				cameraHeight -= speed;
				break;
        }
		std::cout <<"x= "<< cameraPosition.x <<" y= "<< cameraPosition.y << " z= " << cameraPosition.z << "\n";
    }
    
    void Camera::rotate(float pitch, float yaw)
    {
		glm::vec3 newDirection;
		newDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newDirection.y = sin(glm::radians(pitch));
		newDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->cameraDirection = glm::normalize(newDirection);
		//this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    
}
