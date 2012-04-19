(function ($) {

	$(function(){

		if(window.location.hash == '#existing'){
			$('.client-add-form').hide();
			$('.client-existing-form').show();
		}

		$('.show-client-add-form').click(function(){
			$('.client-add-form').show();
			$('.client-existing-form').hide();
		});

		$('.show-client-existing-form').click(function(){
			$('.client-add-form').hide();
			$('.client-existing-form').show();
		});

		$(".form-phone").mask("999-999-9999? x9999");
		$(".form-date").mask("99/99/9999 99:99");
		$(".form-existing-clients").chosen();

		Drupal.jsAC.prototype.hidePopup = function (keycode) {

			// Select item if the right key or mousebutton was pressed.
		  if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
		    // this.input.value = $(this.selected).data('autocompleteValue');
				var selected_value = $(this.selected).text();
				var nid = $(this.selected).data('autocompleteValue');
				$('input[name="existing_business_nid"]').val(nid);
				this.input.value = selected_value;
		  }

			// Hide popup.
			var popup = this.popup;
			if (popup) {
			  this.popup = null;
			  $(popup).fadeOut('fast', function () { $(popup).remove(); });
			}
			this.selected = false;
			$(this.ariaLive).empty();
		}

	});

}(jQuery));

