#pragma once

#include <osg/Group>

#include "commonOsg/osgManagerBase.h"
#include "commonOsg/commonOsg.h"

class OsgManager : public OsgManagerBase
{
public:
	static OsgManager* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new OsgManager();
		}
		return m_pInstance;
	}

	virtual ~OsgManager();

	void updateShow(int nShowType = 0);

protected:
	static OsgManager* m_pInstance;

protected:
	OsgManager();

	osg::ref_ptr<osg::Group> m_pRootGeomDistance = nullptr;
	std::vector<osg::Vec3> m_vPickPoints;
	osg::ref_ptr<osg::Group> m_pMeshGroup = nullptr;

private:
	int m_nDatasetPos = 0;
};
