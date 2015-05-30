
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class BoundigBox
{
public:
	BoundigBox() :
		nullBox(true),
		minVec(),
		maxVec()
	{}

	void expand(const glm::vec3& v)
	{
		if (nullBox)
		{
			nullBox = false;

			minVec = v;
			maxVec = v;
		}
		else
		{
			if (minVec.x > v.x)
                minVec.x = v.x;
            if (minVec.y > v.y)
                minVec.y = v.y;
            if (minVec.z > v.z)
                minVec.z = v.z;

            if (maxVec.x < v.x)
                maxVec.x = v.x;
            if (maxVec.y < v.y)
                maxVec.y = v.y;
            if (maxVec.z < v.z)
                maxVec.z = v.z;
		}
	}

	glm::vec3 getCenter() const
	{
		return (minVec + maxVec) / 2.0f;
	}

	float getRadius() const
	{
		return glm::distance(minVec, maxVec) / 2.0f;
	}

	bool nullBox;
	glm::vec3 minVec, maxVec;
};
