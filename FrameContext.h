#ifndef FRAMECONTEXT_H_ 
#define FRAMECONTEXT_H_

#include "UploadBuffer.h"
#include <unordered_map>

class FrameContext {
public: 
	FrameContext();

	// Each frame requires its own allocator so that it can be reset while another frame is rendering.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdListAlloc = nullptr;

	// Similarly the CBs might change between frames so each frame requires their own CBs
	std::unordered_map<std::string, std::unique_ptr<UploadBuffer>> m_constantBuffers;

	// Each frame keeps its own fence value to check if it can execute or needs to wait
	UINT64 Fence = 0;
};

#endif
