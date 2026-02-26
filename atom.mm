#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <iostream>

using namespace std;

// Delegato per la view Metal
@interface Renderer : NSObject <MTKViewDelegate>
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;
@end

@implementation Renderer
- (instancetype)initWithDevice:(id<MTLDevice>)device {
    if (self = [super init]) {
        _device = device;
        _commandQueue = [_device newCommandQueue];

        static const float vertexData[] = {
            0.0f,  0.5f, 0.0f, 1.0f,
           -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 1.0f,
        };
        _vertexBuffer = [_device newBufferWithBytes:vertexData length:sizeof(vertexData) options:MTLResourceStorageModeShared];

        NSError *error = nil;
        id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
        id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];

        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (!_pipelineState) {
            cerr << "Failed to create pipeline state: " << error.localizedDescription.UTF8String << "\n";
        }
    }
    return self;
}

// Chiamato ogni frame
- (void)drawInMTKView:(MTKView *)view {
    @autoreleasepool {
        id<CAMetalDrawable> drawable = view.currentDrawable;
        if (!drawable) return;

        MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
        if (!renderPassDescriptor) return;

        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.2, 1.0);
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

        [renderEncoder setRenderPipelineState:self.pipelineState];
        [renderEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [renderEncoder endEncoding];

        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
    }
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    // qui potresti aggiornare viewport
}
@end

// Metal shader functions
// Add these at the bottom of the file or in a separate .metal file (here included inline for completeness)


int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];

        NSRect frame = NSMakeRect(0, 0, 800, 600);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:(NSWindowStyleMaskTitled |
                                                                  NSWindowStyleMaskClosable |
                                                                  NSWindowStyleMaskResizable)
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [window makeKeyAndOrderFront:nil];

        // Crea MTKView (view Metal)
        MTKView *mtkView = [[MTKView alloc] initWithFrame:frame];
        [window setContentView:mtkView];

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device) {
            cerr << "Metal non supportato!\n";
            return -1;
        }
        mtkView.device = device;

        Renderer *renderer = [[Renderer alloc] initWithDevice:device];
        mtkView.delegate = renderer;

        [app run]; // loop principale Cocoa
    }
    return 0;
}