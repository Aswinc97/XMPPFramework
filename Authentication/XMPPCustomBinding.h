#import <Foundation/Foundation.h>
// @import KissXML;
#import "KissXML.h"


typedef NS_ENUM(NSInteger, XMPPBindResult) {
	
	XMPPBindResultContinue,      // The custom binding process is still ongoing.
	
	XMPPBindResultSuccess,       // Custom binding succeeded.
	                         // The stream should continue normal post-binding operation.
	
	XMPPBindResultFailFallback, // Custom binding failed.
	                         // The stream should fallback to the standard binding protocol.
	
	XMPPBindResultFailAbort     // Custom binding failed.
	                         // The stream must abort the binding process.
	                         // Further, because the stream is in a bad state (authenticated, but
	                         // unable to complete the full handshake) it must immediately disconnect.
	                         // The given NSError will be reported via xmppStreamDidDisconnect:withError:
};

// Legacy fallback for external modules
#define XMPP_BIND_CONTINUE XMPPBindResultContinue
#define XMPP_BIND_SUCCESS XMPPBindResultSuccess
#define XMPP_BIND_FAIL_FALLBACK XMPPBindResultFailFallback
#define XMPP_BIND_FAIL_ABORT XMPPBindResultFailAbort

/**
 * Binding a JID resource is a standard part of the authentication process,
 * and occurs after SASL authentication completes (which generally authenticates the JID username).
 * 
 * This protocol may be used if there is a need to customize the binding process.
 * For example:
 * 
 * - Custom SASL authentication scheme required both username & resource
 * - Custom SASL authentication scheme provided required resource in server response
 * - Stream Management (XEP-0198) replaces binding with resumption from previously bound session
 * 
 * A custom binding procedure may be plugged into an XMPPStream instance via the delegate method:
 * - (id <XMPPCustomBinding>)xmppStreamWillBind;
**/
NS_ASSUME_NONNULL_BEGIN
@protocol XMPPCustomBinding <NSObject>
@required

/**
 * Attempts to start the custom binding process.
 *
 * If it isn't possible to start the process (perhaps due to missing information),
 * this method should return XMPPBindResultFailFallback or XMPPBindResultFailAbort.
 *
 * (The error message is only used by xmppStream if this method returns XMPPBindResultFailAbort.)
 * 
 * If binding isn't needed (for example, because custom SASL authentication already handled it),
 * this method should return XMPPBindResultSuccess.
 * In this case, xmppStream will immediately move to its post-binding operations.
 *
 * Otherwise this method should send whatever stanzas are needed to begin the binding process.
 * And then return XMPPBindResultContinue.
 *
 * This method is called by automatically XMPPStream.
 * You MUST NOT invoke this method manually.
**/
- (XMPPBindResult)start:(NSError **)errPtr;

/**
 * After the custom binding process has started, all incoming xmpp stanzas are routed to this method.
 * The method should process the stanza as appropriate, and return the coresponding result.
 * If the process is not yet complete, it should return XMPPBindResultContinue,
 * meaning the xmpp stream will continue to forward all incoming xmpp stanzas to this method.
 *
 * This method is called automatically by XMPPStream.
 * You MUST NOT invoke this method manually.
**/
- (XMPPBindResult)handleBind:(NSXMLElement *)auth withError:(NSError **)errPtr;

@optional

/**
 * Optionally implement this method to override the default behavior.
 * By default behavior, we mean the behavior normally taken by xmppStream, which is:
 *
 * - IF the server includes <session xmlns='urn:ietf:params:xml:ns:xmpp-session'/> in its stream:features
 * - AND xmppStream.skipStartSession property is NOT set
 * - THEN xmppStream will send the session start request, and await the response before transitioning to authenticated
 * 
 * Thus if you implement this method and return YES, then xmppStream will skip starting a session,
 * regardless of the stream:features and the current xmppStream.skipStartSession property value.
 * 
 * If you implement this method and return NO, then xmppStream will follow the default behavior detailed above.
 * This means that, even if this method returns NO, the xmppStream may still skip starting a session if
 * the server doesn't require it via its stream:features,
 * or if the user has explicitly forbidden it via the xmppStream.skipStartSession property.
 *
 * The default value is NO.
**/
- (BOOL)shouldSkipStartSessionAfterSuccessfulBinding;

@end
NS_ASSUME_NONNULL_END
