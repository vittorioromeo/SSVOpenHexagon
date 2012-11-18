#include "HSL.h"

namespace hg
{
	HSL::HSL()
		:Hue(0)
		,Saturation(0)
		,Luminance(0)
	{}

	HSL::HSL(int H, int S, int L)
	{
		/** Standard used range is (360, 100, 100), you may change it if you like.
		 *  In the given case you have to make all the proper changes to the convert function.
		 */
		/// Range control for Hue.
		if (H <= 360 && H >= 0)
		{
			Hue = H;
		}
		else
		{
			if(H > 360)
			{
				Hue = H%360;
			}
			else if(H < 0 && H > -360)
			{
				Hue = -H;
			}
			else if(H < -360)
			{
				Hue = -(H % 360);
			}
		}

		/// Range control for Saturation.
		if (S <= 100 && S >= 0)
		{
			Saturation = S;
		}
		else
		{
			if(S > 100)
			{
				Saturation = S%100;
			}
			else if(S < 0 && S > -100)
			{
				Saturation = -S;
			}
			else if(S < -100)
			{
				Saturation = -(S % 100);
			}
		}

		/// Range control for Luminance
		if (L <= 100 && L >= 0)
		{
			Luminance = L;
		}
		else
		{
			if(L > 100)
			{
				Luminance = L % 100;
			}
			if(L < 0 && L > -100)
			{
				Luminance = -L;
			}
			if(L < -100)
			{
				Luminance = -(L % 100);
			}
		}
	}

	float HSL::HueToRGB(float arg1, float arg2, float H)
	{
		if ( H < 0 ) H += 1;
		if ( H > 1 ) H -= 1;
		if ( ( 6 * H ) < 1 )
		{
			return (arg1 + ( arg2 - arg1 ) * 6 * H);
		}
		if ( ( 2 * H ) < 1 )
		{
			return arg2;
		}
		if ( ( 3 * H ) < 2 )
		{
			return ( arg1 + ( arg2 - arg1 ) * ( ( 2 / 3 ) - H ) * 6 );
		}
		return arg1;
	}

	sf::Color HSL::TurnToRGB()
	{
		/// Reconvert to range [0,1]
		float H = (float)Hue / 360;
		float S = (float)Saturation / 100;
		float L = (float)Luminance / 100;
		float arg1, arg2;

		if (S <= 0.0001f)   /// float comparison annoyance.
		{
			sf::Color C(L*255, L*255, L*255);
			return C;
		}
		else
		{
			if ( L < 0.5 )
			{
				arg2 = L * ( 1 + S );
			}
			else
			{
				arg2 = ( L + S ) - ( S * L );
			}
			arg1 = 2 * L - arg2;

			sf::Uint8 r =( 255 * HueToRGB( arg1, arg2, (H + (float)1/3 ) ) );
			sf::Uint8 g =( 255 * HueToRGB( arg1, arg2, H ) );
			sf::Uint8 b =( 255 * HueToRGB( arg1, arg2, (H - (float)1/3 ) ) );

			sf::Color C(r,g,b);
			return C;
		}
	}

	HSL TurnToHSL(const sf::Color& C)
	{
		/// Trivial cases
		/** Note that you can add more than these, like one for each of the basic colors,
		  but that's up to you and how you wish to use the class. */

		if(C == sf::Color::White)
		{
			return HSL(0, 0, 100);
		}
		/// Even if these colors had hue, as they have no saturation they'd remain the same

		if(C == sf::Color::Black)
		{
			return HSL(0, 0, 0);    /// Saturation 100%, Luminance 0%
		}

		float R, G, B;
		R = (float)C.r/255;  /// You need that cast there. period.
		G = (float)C.g/255;
		B = (float)C.b/255;

		/// Non trivial cases.
		float max, min, l, s{0};

		/// Maximum
		max = std::max(std::max(R,G),B);

		/// Minimum
		min = std::min(std::min(R,G),B);

		HSL A;
		l = ((max + min)/2);  /// Lightness/Luminance calculation.

		if (max - min <= 0.0001f )  ///EPSILON is a macro for 0.0001 or however precision you want.
			///Needed for the closest thing to an equality comparison between floats.
		{
			A.Hue = 0;
			A.Saturation = 0;
		}
		else
		{
			float diff = max - min;

			if(A.Luminance < .5)
			{
				s = diff/(max + min);
			}
			else
			{
				s = diff/(2 - max - min);
			}

			float diffR = ( (( max - R ) * 60) + (diff/2) ) / diff;
			float diffG = ( (( max - G ) * 60) + (diff/2) ) / diff;
			float diffB = ( (( max - B ) * 60) + (diff/2) ) / diff;

			/** 60 and 360 are values that can vary.
			  * It depends if you want the range in 360 or anything other than that.
			  * If you change the value then it must be a multiple of six. 360/6 = 60.
			  * That's where the 60 comes from.
			  * The values below are just the range you want the hue in.
			  */
			if (max - R <= 0.0001f)
			{
				A.Hue = diffB - diffG;
			}
			else if ( max - G <= 0.0001f )
			{
				A.Hue = (1*360)/3 + (diffR - diffB);
			}
			else if ( max - B <= 0.0001f )
			{
				A.Hue = (2*360)/3 + (diffG - diffR);
			}

			if (A.Hue < 0)
			{
				A.Hue += 360;
			}
			else if(A.Hue > 360)
			{
				A.Hue -= 360;
			}

			s *= 100;
			/** Saturation and lightness are given in a [0 , 1] interval.
			  * Needed to get it in the right percentage.
			  * Then again, up to you, you may change it if you want. */
		}

		l *= 100;
		A.Saturation = s;
		A.Luminance = l;
		return A;
	}

	// This function extracts the hue, saturation, and luminance from "color"
	// and places these values in h, s, and l respectively.
	HSL RGBtoHSL(sf::Color mColor)
	{
		double r = mColor.r/255.0;
		double g = mColor.g/255.0;
		double b = mColor.b/255.0;
		double v;
		double m;
		double vm;
		double r2, g2, b2;

		double h = 0; // default to black
		double s = 0;
		double l = 0;
		v = max(r,g);
		v = max(v,b);
		m = min(r,g);
		m = min(m,b);
		l = (m + v) / 2.0;
		if (l <= 0.0)
		{
			return HSL(h, s, l);
		}
		vm = v - m;
		s = vm;
		if (s > 0.0)
		{
			s /= (l <= 0.5) ? (v + m ) : (2.0 - v - m) ;
		}
		else
		{
			return HSL(h, s, l);
		}
		r2 = (v - r) / vm;
		g2 = (v - g) / vm;
		b2 = (v - b) / vm;
		if (r == v)
		{
			  h = (g == m ? 5.0 + b2 : 1.0 - g2);
		}
		else if (g == v)
		{
			  h = (b == m ? 1.0 + r2 : 3.0 - b2);
		}
		else
		{
			  h = (r == m ? 3.0 + g2 : 5.0 - r2);
		}
		h /= 6.0;

		return HSL(h * 360.0, s * 100.0, l * 100.0);
	}
}
