#include "IEAsset.hpp"

void IEAsset::addAspect(IEAspect *aspect) {
	aspects.emplace_back(aspect);
	aspect->associatedAssets.push_back(this);
}

IEAsset::~IEAsset() {
	for (IEAspect *aspect: aspects) {
		aspect->associatedAssets.erase(std::find(aspect->associatedAssets.begin(), aspect->associatedAssets.end(), this));
		aspect->destroy();
	}
	if (allAssets) {
		allAssets->erase(allAssets->begin() + (ssize_t) index);
	}
	filename = "";
}

void IEAsset::update(uint32_t renderCommandBufferIndex) {
	for (IEAspect *aspect: aspects) {
		aspect->update(renderCommandBufferIndex);
	}
}
