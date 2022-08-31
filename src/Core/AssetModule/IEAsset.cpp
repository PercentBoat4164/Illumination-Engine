#include "IEAsset.hpp"

void IEAsset::addAspect(IEAspect *aspect) {
	aspects.emplace_back(aspect);
	aspect->associatedAssets.push_back(weak_from_this());
}
