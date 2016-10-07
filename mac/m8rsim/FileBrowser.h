//
//  FileBrowser.h
//  m8rsim
//
//  Created by Chris Marrin on 6/24/16.
//  Copyright © 2016 Chris Marrin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Document;
@class Device;

@interface FileBrowser : NSViewController

- (instancetype)initWithDocument:(Document*) document;

- (void)addFiles;
- (void)removeFiles;
- (BOOL)isFileSourceLocal;

- (void)reloadFilesForDevice:(Device*)device;
- (NSString*)getNewDeviceName;
- (void)addDevice:(NSString*)name;

@end

